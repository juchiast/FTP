#include "net.hpp"
#include <iostream>

int main() {
    try {
        auto listener = net::TcpListener::bind(10000);
    } catch (const char *e) {
        std::cout << e << std::endl;
    }
    return 0;
}
