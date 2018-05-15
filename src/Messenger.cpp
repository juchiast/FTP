#include "net.hpp"
#include <iostream>
#include <string.h>
#define LEN_BUFFER 100

const char CRLF[] = "\r\n";

namespace net {

Messenger::Messenger(const TcpStream &ts) : stream(ts) {}

void Messenger::send(const std::string &msg) {
    stream.write(msg.c_str(), msg.length());
    stream.write(CRLF, 2);
}

// return 0 if not exits cmd
int Messenger::len_first_cmd() {
    size_t p = 0;
    while (p < buff.size() && buff[p] != '\n')
        ++p;
    if (p == buff.size())
        p = -1;
    return p + 1;
}

std::string Messenger::receive() {
    char tmp[LEN_BUFFER + 2];

    while (len_first_cmd() == 0) {
        memset(tmp, 0, sizeof(tmp));
        stream.read(tmp, LEN_BUFFER);
        buff += std::string(tmp);
    }

    int x = len_first_cmd();
    std::string ans = buff.substr(0, x - 2);
    buff.erase(0, x);

    return ans;
}

Messenger::~Messenger() {}
} // namespace net