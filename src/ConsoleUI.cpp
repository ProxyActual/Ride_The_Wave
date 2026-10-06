#include "ConsoleUI.h"
#include <algorithm>
#include <iostream>


void ConsoleUI::drawGraph(int row, int col, int height, int width, const std::vector<double>& data) {

    if (data.empty() || height <= 0 || width <= 0) return;

    double maxValue = *std::max_element(data.begin(), data.end());
    double minValue = *std::min_element(data.begin(), data.end());

    double range = maxValue - minValue;
    if (range == 0) range = 1; // Prevent division by zero
    for(int x = 0; x < width; ++x) {
        double dataPosition = width > 1
            ? static_cast<double>(x) * (data.size() - 1) / (width - 1)
            : 0.0;
        std::size_t dataIndex = static_cast<std::size_t>(dataPosition);
        std::size_t nextIndex = std::min(dataIndex + 1, data.size() - 1);
        double slope = data[nextIndex] - data[dataIndex];
        double value = data[dataIndex] + slope * (dataPosition - dataIndex);

        int barHeight = static_cast<int>((value - minValue) / range * height);
        setCursorPosition(row + height - barHeight, col + x); 
        std::cout << "#";
        std::cout << std::endl;
    }

    setCursorPosition(row + height + 1, 1);
}