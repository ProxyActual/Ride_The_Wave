#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include "Investment.h"

std::vector<float> costHistory;

int main() {
    Investment myInvestment;
    Investment::dataPoint newData;
    newData.buyCost = 100.0f;
    newData.sellCost = 120.0f;
    newData.timestamp = std::chrono::system_clock::now();
    myInvestment.addDataPoint(newData);
    std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> timestampOffset(1, 86400);
    std::uniform_real_distribution<float> costChange(-1.0f, 1.0f);
    float lastSellCost = newData.sellCost;
    for(int i = 0; i < 10000; i++) {
        newData.buyCost = lastSellCost;
        newData.sellCost = lastSellCost + costChange(generator);
        lastSellCost = newData.sellCost;

        newData.timestamp +=
            std::chrono::minutes(1);
        myInvestment.addDataPoint(newData);
    }
    myInvestment.exportHistoryToCSV("investment_history.csv");

}