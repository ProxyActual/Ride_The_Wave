#pragma once
#include <iostream>
#include <vector>
class ConsoleUI {
public:
    

    void clearScreen() {
        std::cout << "\033[2J\033[1;1H";
    }

    void clearLine(int lineUpFromBottom) {
        std::cout << "\033[" << lineUpFromBottom << "A\033[2K";
    }

    void setCursorPosition(int row, int col) {
        std::cout << "\033[" << row << ";" << col << "H";
    }

    void setColor(const std::vector<double>& color) {
        std::cout << "\033[38;2;" << static_cast<int>(color[0]) << ";" << static_cast<int>(color[1]) << ";" << static_cast<int>(color[2]) << "m";
    }

    void drawGraph(int row, int col, int height, int width, const std::vector<double>& data);

    void clearLine(){
        std::cout << "\033[2K";
    }

    std::string getInputs(){
        std::string input;
        std::streambuf* buffer = std::cin.rdbuf();
        while (buffer->in_avail() > 0) {
            std::cout << "INPUT READY: " << std::flush;
            const int next = buffer->sbumpc();
            if (next == std::char_traits<char>::eof() || next == '\n') {
                break;
            }
            input += static_cast<char>(next);
        }
        return input;
    }
};