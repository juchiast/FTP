#include "main.hpp"
#include "net.hpp"
#include <iostream>
#include <thread>

int main() {
    try {
       readCommand();
        
    } catch (const char *e) {
        std::cout << e << std::endl;
    }
    return 0;
}