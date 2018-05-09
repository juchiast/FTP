#include "net.hpp"
#include <string.h>
#define LEN_BUFFER 100

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
    size_t p = 0;
    while (p < buff.size() && buff[p] != '\n');
    if (p == buff.size())
        p = -1;
    return p;
}

std::string Messenger::receive() {
    char tmp[LEN_BUFFER + 2];
    while (pos_first_cmd() == -1) {
        streamfd.read(tmp, LEN_BUFFER);
        buff += std::string(tmp);
    }
    
    return buff.substr(0, pos_first_cmd());
}

Messenger::~Messenger() {
}
}