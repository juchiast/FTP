#include "net.hpp"
#include <iostream>

int main() {
    auto stream = net::TcpStream("10.1.1.100", 21);
    for (char c; stream.read(&c, 1);) {
        std::cout << c;
    }
    return 0;
}