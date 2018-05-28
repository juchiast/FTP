#include "net.hpp"
#include <iostream>
#include <string.h>
#define LEN_BUFFER 100

const char CRLF[] = "\r\n";

namespace net {

/*
 * FTP use <CRLF> to end a message (either a command or reply).
 *
 * This class provides functions to send and receive messages.
 *
 * All functions may throw because of network errors.
 * */
Messenger::Messenger(const TcpStream &ts) : stream(ts) {}

/*
 * Send a message.
 * <CRLF> is automatically appended to the message.
 * */
void Messenger::send(const std::string &msg) {
    std::cout << "-> " + msg << std::endl;
    auto buf = msg + CRLF;
    stream.write(buf.c_str(), buf.length());
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

/*
 * Receive a message.
 * <CRLF> is automatically removed.
 * */
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

Address Messenger::get_listen_address() {
    return this->stream.get_listen_address();
}

Messenger::~Messenger() {}
} // namespace net
