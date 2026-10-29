#include "CoinPredictor.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

CoinPredictor::Evaluation scoreConstant(double prediction, const std::vector<double>& targets) {
    Node constant;
    constant.setValue(prediction);
    Network network(CoinPredictor::WINDOW_DAYS, 0);
    auto* output = new Node();
    network.addNode(output, Network::OUTPUT);
    network.addConnection(output, &constant, 1.0);
    std::vector<double> history(CoinPredictor::WINDOW_DAYS, 0.0);
    history.insert(history.end(), targets.begin(), targets.end());
    return CoinPredictor::scoreNetwork(network, history);
}

bool squaredErrorTest() {
    struct Example {
        double prediction;
        double target;
        double error;
    };
    for (const auto& example : std::vector<Example>{
             {1.0, 1.5, 0.5}, {1.0, 0.5, 0.5}, {-1.0, 1.5, 2.5},
             {1.0, -1.5, 2.5}, {-1.0, -1.5, 0.5}, {-1.0, -0.5, 0.5},
             {0.0, 0.0, 0.0}, {1.5, 1.5, 0.0}}) {
        const auto evaluation = scoreConstant(example.prediction, {example.target});
        if (!evaluation.isValid() || evaluation.samples != 1 ||
            evaluation.meanAbsoluteError != example.error ||
            evaluation.meanSquaredError != example.error * example.error) {
            std::cerr << "Incorrect error for estimate " << example.prediction
                      << " and target " << example.target << '\n';
            return false;
        }
    }

    const auto evaluation = scoreConstant(1.0, {1.5, 0.5, -1.5});
    return evaluation.samples == 3 && evaluation.correctDirections == 2 &&
        std::abs(evaluation.meanAbsoluteError - 3.5 / 3.0) < 1e-12 &&
        std::abs(evaluation.meanSquaredError - 6.75 / 3.0) < 1e-12 &&
        std::abs(evaluation.directionAccuracy - 200.0 / 3.0) < 1e-12;
}

bool selectionTest() {
    const auto accurate = scoreConstant(-0.1, {0.1});
    const auto rightDirection = scoreConstant(2.0, {0.1});
    const auto perfect = scoreConstant(0.1, {0.1});
    const auto tied = scoreConstant(0.3, {0.5});
    CoinPredictor::Evaluation invalid;
    auto nonfinite = accurate;
    nonfinite.meanSquaredError = std::numeric_limits<double>::quiet_NaN();
    const auto outlierMiss = scoreConstant(0.0, {0.0, 0.0, 6.0});
    const auto balanced = scoreConstant(2.0, {0.0, 0.0, 6.0});
    const auto overflow = scoreConstant(1e200, {0.0});

    return outlierMiss.meanAbsoluteError < balanced.meanAbsoluteError &&
        outlierMiss.meanSquaredError == 12.0 && balanced.meanSquaredError == 8.0 &&
        balanced.isBetterThan(outlierMiss) && !outlierMiss.isBetterThan(balanced) &&
        !overflow.isValid() && !overflow.isBetterThan(accurate) &&
        accurate.correctDirections == 0 && rightDirection.correctDirections == 1 &&
        accurate.isBetterThan(rightDirection) && !rightDirection.isBetterThan(accurate) &&
        perfect.isBetterThan(accurate) && !accurate.isBetterThan(perfect) &&
        !accurate.isBetterThan(tied) && !tied.isBetterThan(accurate) &&
        !perfect.isBetterThan(perfect) && accurate.isBetterThan(invalid) &&
        !invalid.isBetterThan(accurate) && !invalid.isBetterThan(invalid) &&
        !nonfinite.isBetterThan(accurate) && accurate.isBetterThan(nonfinite);
}

int main() {
    if (!squaredErrorTest()) {
        std::cerr << "Squared error scoring test failed\n";
        return 1;
    }
    if (!selectionTest()) {
        std::cerr << "Lowest error selection test failed\n";
        return 1;
    }
    std::cout << "Squared error scoring and lowest error selection tests passed\n";
}
