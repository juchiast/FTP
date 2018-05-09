#include "net.hpp"
#include <iostream>

int main() {
    try {
        auto listener = net::TcpListener::bind(21);
        auto stream1 = net::TcpStream::connect("127.0.0.1", 21);
        auto stream2 = listener.next();
        return 0;

        char buff1[] = "AHJHJ";
        stream1.write(buff1, 5);
        char buff2[100];
        stream2.read(buff2, 5);

        std::cout << buff2 << std::endl;
    } catch (const char *e) {
        std::cout << e << std::endl;
    }
    return 0;
}