#pragma once

#include <chrono>
#include <vector>
#include <string>

class Investment {
    public:
        Investment() {}

        struct dataPoint{
            float sellCost;
            float buyCost;
            std::chrono::time_point<std::chrono::system_clock> timestamp;
        };

        std::vector<dataPoint> history;

        void addDataPoint(dataPoint newData);
        void clearHistory();
        void printHistory();

        void exportHistoryToCSV(const std::string& filename);
};