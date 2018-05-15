#include "net.hpp"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
using namespace net;

static struct sockaddr_in __get_address(const char *ip, uint16_t port) {
    uint32_t x = 0;
    int err_code = inet_pton(AF_INET, ip, &x);
    if (err_code == 0) {
        throw "Invalid address string";
    } else if (err_code == -1) {
        throw strerror(errno);
    }
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = x;
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
    return addr;
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
    return from_raw(__connect(__get_address(remote_ip, remote_port)));
}

size_t TcpStream::read(void *buffer, size_t count) {
    // TODO what happen when remote close the connection
    auto r = recv(this->sockfd, buffer, count, 0);
    if (r < 0) {
        throw strerror(errno);
    }
    return r;
}

size_t TcpStream::write(const void *buffer, size_t count) {
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
    shutdown(this->sockfd, SHUT_RDWR);
}

TcpStream::~TcpStream() { this->close_both(); }

TcpStream TcpStream::from_raw(int _sockfd) {
    TcpStream t;
    t.sockfd = _sockfd;
    return t;
}

TcpStream &TcpStream::operator=(const TcpStream &t) {
    this->close_both();
    this->sockfd = dup(t.sockfd);
    if (this->sockfd == -1) {
        throw strerror(errno);
    }
    return *this;
}

TcpStream::TcpStream(const TcpStream &t) {
    this->sockfd = dup(t.sockfd);
    if (this->sockfd == -1) {
        throw strerror(errno);
    }
}
} // namespace net