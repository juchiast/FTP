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
}

void Messenger::receive() {
    int x = has_command();

}

Messenger::~Messenger() {
}
}