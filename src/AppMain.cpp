#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "ConfigManager.h"

int main(){
    std::cout << "Hello, Ride The Wave!" << std::endl;

    ConfigManager configManager;
    configManager.loadSettings("config.txt");

    

}