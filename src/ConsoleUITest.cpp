#include "ConsoleUI.h"

#include <iostream>
#include <thread>
#include <chrono>

int main() {
    ConsoleUI ui;
    
    ui.clearScreen();

    std::string input;

    while (true) {
        ui.setCursorPosition(10, 3);
        std::cout << "Hello, World! "<< input << std::flush;
        ui.setCursorPosition(40, 30);
        std::cout << std::flush;
        std::cout << "Enter input: " << std::flush;
        input = ui.getInputs();
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    return 0;
}