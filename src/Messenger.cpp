#include "net.hpp"
#include <string.h>

namespace net {

Messenger::Messenger(const TcpStream& ts) {
    streamfd = ts;
    buff = "";
}

void Messenger::send(void* buffer, int count) {
    char* tmp = new char[count + 2];
    memcpy(tmp, buffer, count);
    tmp[count] = '\r';
    tmp[count + 1] = '\n';
    streamfd.write(tmp, count + 2);
    delete[] tmp;
}

int Messenger::pos_first_cmd() {
    static size_t p = 0;
    while (p < buff.size() && buff[p] != '\n');
    if (p == buff.size())
        p = -1;
    return p;
}

std::string Messenger::receive() {
    // int x = has_command();
    return "";
}

Messenger::~Messenger() {
}
}