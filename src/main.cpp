#include "net.hpp"
#include <iostream>

int main() {
    try {
        auto stream_client = net::TcpStream::connect("10.1.1.100", 21);
        auto mess_client = net::Messenger(stream_client);

        char name[] = "USER user";
        char passwd[] = "PASS pass";

        mess_client.send(name);
        mess_client.send(passwd);
        mess_client.send("FEAT");
        while (1) {
            std::cout << mess_client.receive() << std::endl;
        }
    } catch (const char *e) {
        std::cout << e << std::endl;
    }
    return 0;
}
