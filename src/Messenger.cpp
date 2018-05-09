#include "net.hpp"
#include <string.h>
#include <iostream>
#define LEN_BUFFER 100

namespace net {

Messenger::Messenger(const TcpStream& ts) {
    streamfd = ts;
    buff = "";
}

void Messenger::send(const void* buffer, int count) {
    char* tmp = new char[count + 2];
    memcpy(tmp, buffer, count);
    tmp[count] = '\r';
    tmp[count + 1] = '\n';
    streamfd.write(tmp, count + 2);
    delete[] tmp;
}

//return 0 if not exits cmd
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
        streamfd.read(tmp, LEN_BUFFER); 
        buff += std::string(tmp);
    }
    
    int x = len_first_cmd();
    std::string ans = buff.substr(0, x - 2);
    buff.erase(0, x);

    return ans;
}

Messenger::~Messenger() {
}
}