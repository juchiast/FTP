#include "net.hpp"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static struct sockaddr_in __get_address(uint16_t local_port) {
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(local_port);
    addr.sin_family = AF_INET;
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
    return addr;
}

static int __bind(struct sockaddr_in addr) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        throw strerror(errno);
    }
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        throw strerror(errno);
    }
    return sock;
}

namespace net {
TcpListener::~TcpListener() {
    close(this->listenfd);
}
TcpListener TcpListener::bind(uint16_t local_port) {
    TcpListener t;
    t.listenfd = __bind(__get_address(local_port));
    if (listen(t.listenfd, 10000) == -1) {
        throw strerror(errno);
    }
    return t;
}

TcpStream TcpListener::next() {
    int sockfd = accept(this->listenfd, nullptr, nullptr);
    if (sockfd < 0) {
        throw strerror(errno);
    }
    return TcpStream::from_raw(sockfd);
}
} // namespace net