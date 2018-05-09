#include "net.hpp"
#include <iostream>

int main() {
    try {
        auto stream1 = net::TcpStream::connect("127.0.0.1", 21);

        char passwd[] = "PASS 366117187\r\n";
        char name[] = "USER thiennu\r\n";

        char buf[200];
        int len;

        stream1.read(buf, 200);
        std::cout << buf << std::endl;

        stream1.write(name, sizeof(name)-1);
        len = stream1.read(buf, 200);
        buf[len] = 0;
        std::cout << buf << std::endl;

        stream1.write(passwd, sizeof(passwd)-1);
        len = stream1.read(buf, 200);
        buf[len] = 0;
        std::cout << buf << std::endl;
    } catch (const char *e) {
        std::cout << e << std::endl;
    }
    return 0;
}