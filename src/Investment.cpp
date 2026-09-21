#include "Investment.h"
#include <iostream>
#include <fstream>

void Investment::addDataPoint(dataPoint newData) {
    if(history.empty()){
        history.push_back(newData);
    } else if (newData.timestamp > history.back().timestamp) {
        history.push_back(newData);
    } else if (newData.timestamp < history.front().timestamp) {
        history.insert(history.begin(), newData);
    } else {
        int position = 0;
        for(dataPoint index: history) {
            if(newData.timestamp < index.timestamp) {
                break;
            }
            position++;
        }
        history.insert(history.begin() + position, newData);
    }
}

void Investment::clearHistory() {
    history.clear();
}

void Investment::printHistory() {
    std::cout << "TimeStamp,Buy,Sell" << std::endl;
    for(const dataPoint& dp : history) {
        const auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            dp.timestamp.time_since_epoch()).count();

        std::cout << timestamp << "," << dp.buyCost << "," << dp.sellCost << std::endl;
    }
}

void Investment::exportHistoryToCSV(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }
    file << "TimeStamp,Buy,Sell" << std::endl;
    for(const dataPoint& dp : history) {
        const auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            dp.timestamp.time_since_epoch()).count();
        file << timestamp << "," << dp.buyCost << "," << dp.sellCost << std::endl;
    }
    file.close();
}

