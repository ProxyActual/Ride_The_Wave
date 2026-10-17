#pragma once
#include <vector>
#include <string>

#include "ConsoleUI.h"

class ConsoleWidget {
    public:
        struct Color {
            int r_;
            int g_;
            int b_;
        };

        static constexpr Color DEFAULT{-1, -1, -1};
        static constexpr Color WHITE{255, 255, 255};
        static constexpr Color RED{255, 0, 0};
        static constexpr Color GREEN{0, 255, 0};
        static constexpr Color BLUE{0, 0, 255};
        static constexpr Color BLACK{0, 0, 0};


        ConsoleWidget(int x, int y, int width, int height);
        ~ConsoleWidget();

        void update();

        void forceUpdate();

        void putText(int x, int y, const std::string& text, const Color& fgColor, const Color& bgColor);

        void clear(int x, int y, int width, int height);

        int getX() const { return x_; }
        int getY() const { return y_; }
        int getWidth() const { return width_; }
        int getHeight() const { return height_; }

    private:
        std::vector<std::vector<std::string>> buffer_{};
        std::vector<std::vector<std::string>> newBuffer_{};
        int x_, y_, width_, height_;

        ConsoleUI consoleUI_;
};