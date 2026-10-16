
#include "ConsoleWidget.h"
#include <iostream>

ConsoleWidget::ConsoleWidget(int x, int y, int width, int height) 
    : x_(x), y_(y), width_(width), height_(height), consoleUI_() {
    std::vector<std::string> emptyRow;
    for(int i = 0; i < width_; ++i) {
        emptyRow.push_back(" ");
    }
    for(int i = 0; i < height_; ++i) {
        buffer_.push_back(emptyRow);
        newBuffer_.push_back(emptyRow);
    }
}

ConsoleWidget::~ConsoleWidget() {
    // No manual deletion needed as we are using std::vector
}

void ConsoleWidget::putText(int x, int y, const std::string& text, const Color& fgColor, const Color& bgColor) {
    for (int i = 0; i < text.size(); ++i) {
        if(x + i < width_ && y < height_) {
            newBuffer_[y][x + i] = "";
            std::string fgColorCode = "\033[38;2;" + std::to_string(fgColor.r_) + ";" + std::to_string(fgColor.g_) + ";" + std::to_string(fgColor.b_) + "m";
            std::string bgColorCode = "\033[48;2;" + std::to_string(bgColor.r_) + ";" + std::to_string(bgColor.g_) + ";" + std::to_string(bgColor.b_) + "m";

            if(fgColor.r_ >= 0 && fgColor.g_ >= 0 && fgColor.b_ >= 0) {
                newBuffer_[y][x + i] += fgColorCode;
            }
            if(bgColor.r_ >= 0 && bgColor.g_ >= 0 && bgColor.b_ >= 0) {
                newBuffer_[y][x + i] += bgColorCode;
            }

            newBuffer_[y][x + i] += text[i];
        }
    }
}

void ConsoleWidget::update() {
    for(int y = 0; y < height_; ++y) {
        for(int x = 0; x < width_; ++x) {
            if(buffer_[y][x] != newBuffer_[y][x]) {
                buffer_[y][x] = newBuffer_[y][x];
                std::cout << "\033[0m"; // Reset color after printing each character
                consoleUI_.setCursorPosition(y + y_, x + x_);
                std::cout << buffer_[y][x];
                std::cout.flush();
            }
        }
    }
}

void ConsoleWidget::clear(int x, int y, int width, int height) {
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            if (x + i < width_ && y + j < height_) {
                newBuffer_[y + j][x + i] = " ";
            }
        }
    }
}

void ConsoleWidget::forceUpdate() {
    for(int y = 0; y < height_; ++y) {
        for(int x = 0; x < width_; ++x) {
            buffer_[y][x] = newBuffer_[y][x];
            consoleUI_.setCursorPosition(y + y_, x + x_);
            std::cout << buffer_[y][x];
            std::cout.flush();
        }
    }
}