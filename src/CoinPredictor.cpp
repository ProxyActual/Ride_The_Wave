#include "CoinPredictor.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace {

std::string quoteForShell(const std::string& value) {
    std::string quoted = "'";
    for (char c : value) {
        if (c == '\'') {
            quoted += "'\\''";
        } else {
            quoted += c;
        }
    }
    return quoted + "'";
}

bool isSafeAssetCode(const std::string& assetCode) {
    if (assetCode.empty()) {
        return false;
    }
    for (unsigned char c : assetCode) {
        if (!std::isalnum(c)) {
            return false;
        }
    }
    return true;
}

std::vector<std::string> splitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;
    while (std::getline(stream, field, ',')) {
        if (!field.empty() && field.back() == '\r') {
            field.pop_back();
        }
        fields.push_back(field);
    }
    return fields;
}

std::int64_t unixNow() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

double randomUnit() {
    return static_cast<double>(std::rand()) / RAND_MAX;
}

} // namespace

CoinPredictor::CoinPredictor(const std::string& assetCode, const Settings& settings)
    : assetCode_(assetCode), settings_(settings)
{
    snapshot_.status = "starting";
    worker_ = std::jthread([this](std::stop_token stopToken) { run(stopToken); });
}

CoinPredictor::~CoinPredictor() {
    worker_.request_stop();
    wakeUp_.notify_all();
}

CoinPredictor::Snapshot CoinPredictor::getSnapshot() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return snapshot_;
}

std::filesystem::path CoinPredictor::getNetworkPath() const {
    return settings_.stateDirectory / (assetCode_ + "_network.txt");
}

std::vector<double> CoinPredictor::loadPercentChanges(const std::filesystem::path& csvPath, std::int64_t nowUnixSeconds) {
    std::vector<double> percentChanges;
    std::ifstream file(csvPath);
    std::string line;
    if (!file || !std::getline(file, line)) {
        return percentChanges;
    }

    const auto header = splitCsvLine(line);
    int dateColumn = -1;
    int changeColumn = -1;
    for (size_t i = 0; i < header.size(); ++i) {
        if (header[i] == "Date") {
            dateColumn = static_cast<int>(i);
        } else if (header[i] == "PercentChange") {
            changeColumn = static_cast<int>(i);
        }
    }
    if (changeColumn < 0) {
        return percentChanges;
    }

    std::int64_t lastDate = 0;
    while (std::getline(file, line)) {
        const auto fields = splitCsvLine(line);
        if (static_cast<int>(fields.size()) <= changeColumn || fields[changeColumn].empty()) {
            continue;
        }
        try {
            const double change = std::stod(fields[changeColumn]);
            if (!std::isfinite(change)) {
                continue;
            }
            percentChanges.push_back(change);
            if (dateColumn >= 0 && dateColumn < static_cast<int>(fields.size())) {
                lastDate = std::stoll(fields[dateColumn]);
            }
        } catch (const std::exception&) {
            continue;
        }
    }

    // Yahoo includes today's still-forming daily candle; it is not a finished day.
    constexpr std::int64_t SECONDS_PER_DAY = 24 * 60 * 60;
    if (!percentChanges.empty() && lastDate > 0 && lastDate + SECONDS_PER_DAY > nowUnixSeconds) {
        percentChanges.pop_back();
    }
    return percentChanges;
}

CoinPredictor::Evaluation CoinPredictor::scoreNetwork(Network& network, const std::vector<double>& percentChanges) {
    Evaluation evaluation;
    if (percentChanges.size() <= static_cast<size_t>(WINDOW_DAYS)) {
        return evaluation;
    }
    double totalError = 0.0;
    double totalSquaredError = 0.0;
    size_t correctDirections = 0;
    size_t samples = 0;
    std::vector<double> inputs(WINDOW_DAYS);
    for (size_t target = WINDOW_DAYS; target < percentChanges.size(); ++target) {
        std::copy(percentChanges.begin() + (target - WINDOW_DAYS), percentChanges.begin() + target, inputs.begin());
        const auto outputs = network.calculate(inputs);
        if (outputs.empty()) {
            return evaluation;
        }
        const double predicted = outputs[0];
        const double actual = percentChanges[target];
        const double error = std::abs(predicted - actual);
        const bool correctDirection = predicted * actual > 0.0;
        totalError += error;
        totalSquaredError += error * error;
        correctDirections += correctDirection ? 1 : 0;
        ++samples;
    }
    const double count = static_cast<double>(samples);
    const double meanAbsoluteError = totalError / count;
    const double meanSquaredError = totalSquaredError / count;
    if (!std::isfinite(meanAbsoluteError) || !std::isfinite(meanSquaredError)) {
        return evaluation;
    }
    evaluation.correctDirections = correctDirections;
    evaluation.samples = samples;
    evaluation.meanAbsoluteError = meanAbsoluteError;
    evaluation.meanSquaredError = meanSquaredError;
    evaluation.directionAccuracy = 100.0 * static_cast<double>(correctDirections) / count;
    return evaluation;
}

double CoinPredictor::predictNext(Network& network, const std::vector<double>& percentChanges) {
    if (percentChanges.size() < static_cast<size_t>(WINDOW_DAYS)) {
        return 0.0;
    }
    std::vector<double> inputs(percentChanges.end() - WINDOW_DAYS, percentChanges.end());
    const auto outputs = network.calculate(inputs);
    return outputs.empty() ? 0.0 : outputs[0];
}

void CoinPredictor::setStatus(const std::string& status) {
    std::lock_guard<std::mutex> lock(mutex_);
    snapshot_.status = status;
}

bool CoinPredictor::writeEstimatedChanges(Network& network, const std::filesystem::path& csvPath,
                                          const std::vector<double>& percentChanges) {
    std::ifstream in(csvPath);
    std::string line;
    if (!in || !std::getline(in, line)) {
        return false;
    }
    auto header = splitCsvLine(line);
    int changeColumn = -1;
    int estimateColumn = -1;
    for (size_t i = 0; i < header.size(); ++i) {
        if (header[i] == "PercentChange") {
            changeColumn = static_cast<int>(i);
        } else if (header[i] == "EstimatedChange") {
            estimateColumn = static_cast<int>(i);
        }
    }
    if (changeColumn < 0) {
        return false;
    }
    if (estimateColumn < 0) {
        header.push_back("EstimatedChange");
        estimateColumn = static_cast<int>(header.size()) - 1;
    }

    const auto join = [](const std::vector<std::string>& fields) {
        std::string out;
        for (size_t i = 0; i < fields.size(); ++i) {
            out += (i ? "," : "") + fields[i];
        }
        return out + "\n";
    };

    const auto tempPath = csvPath.string() + ".tmp";
    {
        std::ofstream out(tempPath, std::ios::trunc);
        if (!out) {
            return false;
        }
        out << join(header);
        out << std::setprecision(17);
        std::vector<double> inputs(WINDOW_DAYS);
        size_t validIndex = 0; // index into percentChanges, which skips unparsable rows
        while (std::getline(in, line)) {
            auto fields = splitCsvLine(line);
            if (fields.empty()) {
                continue;
            }
            fields.resize(std::max(fields.size(), header.size()));
            double estimate = 0.0;
            bool valid = false;
            if (static_cast<int>(fields.size()) > changeColumn && !fields[changeColumn].empty()) {
                try {
                    valid = std::isfinite(std::stod(fields[changeColumn]));
                } catch (const std::exception&) {
                }
            }
            if (valid) {
                // The first WINDOW_DAYS rows have no full window, so they stay 0.
                if (validIndex >= static_cast<size_t>(WINDOW_DAYS) && validIndex <= percentChanges.size()) {
                    std::copy(percentChanges.begin() + (validIndex - WINDOW_DAYS),
                              percentChanges.begin() + validIndex, inputs.begin());
                    const auto outputs = network.calculate(inputs);
                    estimate = outputs.empty() ? 0.0 : outputs[0];
                }
                ++validIndex;
            }
            std::ostringstream value;
            value << std::setprecision(17) << (std::isfinite(estimate) ? estimate : 0.0);
            fields[estimateColumn] = value.str();
            out << join(fields);
        }
        if (!out.good()) {
            return false;
        }
    }
    std::error_code error;
    std::filesystem::rename(tempPath, csvPath, error);
    return !error;
}

void CoinPredictor::publish(Network& best, const Evaluation& bestEvaluation) {
    const double prediction = predictNext(best, percentChanges_);
    writeEstimatedChanges(best, settings_.historyDirectory / (assetCode_ + "-USD_1d.csv"), percentChanges_);
    std::lock_guard<std::mutex> lock(mutex_);
    snapshot_.ready = true;
    snapshot_.status = "training";
    snapshot_.correctDirections = bestEvaluation.correctDirections;
    snapshot_.samples = bestEvaluation.samples;
    snapshot_.meanAbsoluteError = bestEvaluation.meanAbsoluteError;
    snapshot_.directionAccuracy = bestEvaluation.directionAccuracy;
    snapshot_.prediction = prediction;
}

bool CoinPredictor::waitFor(std::stop_token& stopToken, std::chrono::milliseconds duration) {
    std::unique_lock<std::mutex> lock(mutex_);
    wakeUp_.wait_for(lock, stopToken, duration, [] { return false; });
    return !stopToken.stop_requested();
}

bool CoinPredictor::fetchHistory() {
    if (!isSafeAssetCode(assetCode_)) {
        return false;
    }
    std::error_code error;
    std::filesystem::create_directories(settings_.historyDirectory, error);

    const auto csvPath = settings_.historyDirectory / (assetCode_ + "-USD_1d.csv");
    const auto logPath = settings_.historyDirectory / (assetCode_ + "-USD_fetch.log");
    const std::string command =
        quoteForShell(settings_.pythonExecutable) + " " +
        quoteForShell(settings_.historyScript.string()) +
        " --symbol " + assetCode_ + "-USD --period " + HISTORY_PERIOD + " --interval 1d --no-copy" +
        " --output " + quoteForShell(csvPath.string()) +
        " > " + quoteForShell(logPath.string()) + " 2>&1";
    return std::system(command.c_str()) == 0;
}

bool CoinPredictor::refreshData(Network& best, Evaluation& bestEvaluation) {
    setStatus(std::string("fetching ") + HISTORY_PERIOD + " history");
    fetchHistory();

    // If the download failed, fall back to the last CSV we fetched.
    auto percentChanges = loadPercentChanges(
        settings_.historyDirectory / (assetCode_ + "-USD_1d.csv"), unixNow());
    if (percentChanges.size() <= static_cast<size_t>(WINDOW_DAYS)) {
        return false;
    }
    percentChanges_ = std::move(percentChanges);
    bestEvaluation = scoreNetwork(best, percentChanges_);
    publish(best, bestEvaluation);
    return true;
}

void CoinPredictor::run(std::stop_token stopToken) {
    auto best = std::make_unique<Network>(WINDOW_DAYS, 1);
    bool loadedFromFile = false;
    const auto networkPath = getNetworkPath();
    if (std::filesystem::exists(networkPath)) {
        Network fromFile(0, 0);
        if (fromFile.loadFromFile(networkPath.string()) &&
            fromFile.getInputCount() == WINDOW_DAYS && fromFile.getOutputCount() == 1) {
            *best = fromFile;
            loadedFromFile = true;
        }
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        snapshot_.loadedFromFile = loadedFromFile;
    }

    Evaluation bestEvaluation;
    while (!refreshData(*best, bestEvaluation)) {
        setStatus("no history (see " + (settings_.historyDirectory / (assetCode_ + "-USD_fetch.log")).filename().string() + ")");
        if (!waitFor(stopToken, std::chrono::minutes(5))) {
            return;
        }
    }
    if (!loadedFromFile) {
        best->saveToFile(networkPath.string());
    }
    auto lastRefresh = std::chrono::steady_clock::now();

    std::uint64_t generations = 0;
    std::uint64_t improvements = 0;
    while (!stopToken.stop_requested()) {
        if (std::chrono::steady_clock::now() - lastRefresh > settings_.refreshInterval) {
            refreshData(*best, bestEvaluation);
            lastRefresh = std::chrono::steady_clock::now();
        }

        auto candidate = std::make_unique<Network>(*best);
        if (randomUnit() < 0.3) {
            candidate->evolve(0.75, 0.25);
        }
        candidate->mutateWeights(0.2, 0.05);
        const Evaluation candidateEvaluation = scoreNetwork(*candidate, percentChanges_);
        ++generations;

        if (candidateEvaluation.isBetterThan(bestEvaluation)) {
            best = std::move(candidate);
            bestEvaluation = candidateEvaluation;
            ++improvements;
            best->saveToFile(networkPath.string());
            publish(*best, bestEvaluation);
        }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            snapshot_.generations = generations;
            snapshot_.improvements = improvements;
        }

        if (!waitFor(stopToken, std::chrono::milliseconds(2))) {
            break;
        }
    }
}
