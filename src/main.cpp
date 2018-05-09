#include "main.hpp"
#include "net.hpp"
#include <iostream>
#include <thread>

int main() {
    try {
       read_command();
        
    } catch (const char *e) {
        std::cout << e << std::endl;
    }
    return 0;
}