#include "net.hpp"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

static struct sockaddr_in __get_address(const char *addr, uint16_t port) {
    uint32_t x = 0;
    int err_code = inet_pton(AF_INET, addr, &x);
    if (err_code == 0) {
        throw "Invalid address string";
    } else if (err_code == -1) {
        throw strerror(errno);
    }
    struct sockaddr_in server;
    server.sin_addr.s_addr = x;
    server.sin_port = htons(port);
    server.sin_family = AF_INET;
    return server;
}

static int __connect(struct sockaddr_in addr) {
    auto sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        throw strerror(errno);
    }
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        throw strerror(errno);
    }
    return sockfd;
}

namespace net {
TcpStream::TcpStream() {}

TcpStream TcpStream::connect(const char *remote_ip, uint16_t remote_port) {
    TcpStream t;
    t.sockfd = __connect(__get_address(remote_ip, remote_port));
    return t;
}

size_t TcpStream::read(void *buffer, size_t count) {
    // TODO what happen when remote close the connection
    auto r = recv(this->sockfd, buffer, count, 0);
    if (r < 0) {
        throw strerror(errno);
    }
    return r;
}

size_t TcpStream::write(void *buffer, size_t count) {
    // TODO what happen when remote close the connection
    auto r = send(this->sockfd, buffer, count, 0);
    if (r < 0) {
        throw strerror(errno);
    }
    return r;
}


void TcpStream::close_read() {
    if (shutdown(this->sockfd, SHUT_RD) == -1) {
        throw strerror(errno);
    }
}

void TcpStream::close_write() {
    if (shutdown(this->sockfd, SHUT_WR) == -1) {
        throw strerror(errno);
    }
}
void TcpStream::close_both() {
    if (shutdown(this->sockfd, SHUT_RDWR) == -1) {
        throw strerror(errno);
    }
}

TcpStream::~TcpStream() {
    this->close_both();
}
} // namespace net