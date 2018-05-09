#include "net.hpp"
#include <iostream>

int main() {
    try {
        auto stream_client = net::TcpStream::connect("127.0.0.1", 21);
        auto mess_client = net::Messenger(stream_client);

        std::cout << mess_client.receive() << std::endl;

        char name[] = "USER qcuong98";
        char passwd[] = "PASS 071098";

        mess_client.send(name, sizeof(name) - 1);
        std::cout << mess_client.receive() << std::endl;

        mess_client.send(passwd, sizeof(passwd) - 1);
        std::cout << mess_client.receive() << std::endl;

        mess_client.send("FEAT", 4);
        while (1) {
            std::cout << mess_client.receive() << std::endl;
        }
    } catch (const char *e) {
        std::cout << e << std::endl;
    }
    return 0;
}
