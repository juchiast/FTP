#include "net.hpp"
#include <iostream>

int main() {
    try {
        auto listener = net::TcpListener::bind(10000);
        auto stream1 = net::TcpStream::connect("127.0.0.1", 10000);
        auto stream2 = listener.next();
    } catch (const char *e) {
        std::cout << e << std::endl;
    }
    return 0;
}