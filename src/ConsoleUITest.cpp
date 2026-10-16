#include "ConsoleUI.h"
#include "ConsoleWidget.h"

#include <iostream>
#include <thread>
#include <chrono>

int main() {
    ConsoleUI ui;
    
    ui.clearScreen();

    ConsoleWidget widget(1, 1, 80, 24);

    std::string input;

    while (true) {
        widget.update();
        int randomWidth = rand() % 80;
        int randomHeight = rand() % 24;

        widget.putText(randomWidth, randomHeight, "Hello, World!", {255, 255, -1}, {-1, 0, 0});

        widget.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        widget.clear(randomWidth, randomHeight, 13, 1);
    }

    return 0;
}