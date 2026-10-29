#pragma once

#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "NeuralNetwork/Network.h"

// Trains a network per coin in the background. The network looks at the last
// WINDOW_DAYS daily percent changes and predicts the next day's percent change.
// The current best network is kept; a mutated copy replaces it only when it lowers
// the mean squared error over the HISTORY_PERIOD daily history, aiming for zero.
// Up/down direction accuracy is reported separately and does not affect selection.
class CoinPredictor {
    public:
        static constexpr int WINDOW_DAYS = 100;
        static constexpr const char* HISTORY_PERIOD = "3y";
        struct Evaluation {
            std::size_t correctDirections {0};  // days where the up/down call was right
            std::size_t samples {0};
            double meanAbsoluteError {INFINITY};
            double meanSquaredError {INFINITY}; // squared percentage points; training objective
            double directionAccuracy {0.0};     // percent of days with the up/down call right

            bool isValid() const {
                return samples > 0 && std::isfinite(meanAbsoluteError) && std::isfinite(meanSquaredError);
            }
            // Lower squared error wins; equal scores keep the current network.
            bool isBetterThan(const Evaluation& other) const {
                if (!isValid()) {
                    return false;
                }
                return !other.isValid() || meanSquaredError < other.meanSquaredError;
            }
        };

        struct Settings {
            std::filesystem::path stateDirectory;   // where <COIN>_network.txt is saved (next to config)
            std::filesystem::path historyDirectory; // where downloaded CSVs are written
            std::string pythonExecutable;
            std::filesystem::path historyScript;
            std::chrono::hours refreshInterval{6};
        };

        struct Snapshot {
            std::string status;
            bool ready {false};
            bool loadedFromFile {false};
            std::size_t correctDirections {0};
            std::size_t samples {0};
            double meanAbsoluteError {0.0};  // percentage points
            double directionAccuracy {0.0};
            double prediction {0.0};  // predicted next-day percent change
            std::uint64_t generations {0};
            std::uint64_t improvements {0};
        };

        CoinPredictor(const std::string& assetCode, const Settings& settings);
        ~CoinPredictor();

        CoinPredictor(const CoinPredictor&) = delete;
        CoinPredictor& operator=(const CoinPredictor&) = delete;

        Snapshot getSnapshot() const;
        const std::string& getAssetCode() const { return assetCode_; }
        std::filesystem::path getNetworkPath() const;

        // Reads the PercentChange column from a CSV, dropping an incomplete trailing daily candle.
        static std::vector<double> loadPercentChanges(const std::filesystem::path& csvPath, std::int64_t nowUnixSeconds);
        // Averages (predicted - actual)^2 for training; zero is ideal.
        // Also reports mean absolute error in percentage points for display.
        // Scores predicting each day from the previous WINDOW_DAYS days. A day counts as a correct
        // direction only when prediction and actual change share a sign, so a flat 0% guess is wrong.
        static Evaluation scoreNetwork(Network& network, const std::vector<double>& percentChanges);
        // Rewrites the CSV with an EstimatedChange column: 0 for the first WINDOW_DAYS rows, then the
        // network's estimate from the previous WINDOW_DAYS days (the unfinished last row gets the next-day estimate).
        static bool writeEstimatedChanges(Network& network, const std::filesystem::path& csvPath,
                                          const std::vector<double>& percentChanges);
        static double predictNext(Network& network, const std::vector<double>& percentChanges);

    private:
        void run(std::stop_token stopToken);
        bool fetchHistory();
        bool refreshData(Network& best, Evaluation& bestEvaluation);
        void setStatus(const std::string& status);
        void publish(Network& best, const Evaluation& bestEvaluation);
        bool waitFor(std::stop_token& stopToken, std::chrono::milliseconds duration);

        std::string assetCode_;
        Settings settings_;

        std::vector<double> percentChanges_;

        mutable std::mutex mutex_;
        std::condition_variable_any wakeUp_;
        Snapshot snapshot_;

        std::jthread worker_;
};
