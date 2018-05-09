#include "net.hpp"
#include <iostream>

int main() {
    try {
        auto stream_client = net::TcpStream::connect("127.0.0.1", 21);
        auto mess_client = net::Messenger(stream_client);
        char buff2[100];

        stream_client.read(buff2, 100);
        std::cout << buff2 << std::endl;

        char name[] = "USER qcuong98";
        char passwd[] = "PASS 071098";

        mess_client.send(name, sizeof(name) - 1);
        stream_client.read(buff2, 100);
        std::cout << buff2 << std::endl;
        return 0;

        mess_client.send(passwd, sizeof(passwd) - 1);
        stream_client.read(buff2, 100);
        std::cout << buff2 << std::endl;
    } catch (const char *e) {
        std::cout << e << std::endl;
    }
    return 0;
}
