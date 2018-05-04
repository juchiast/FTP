#include "net.hpp"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

static struct sockaddr_in get_address(const char *addr, uint16_t port) {
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

static int connect(struct sockaddr_in addr) {
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
TcpStream::TcpStream(const char *remote_ip, uint16_t remote_port) {
    this->sockfd = connect(get_address(remote_ip, remote_port));
}

size_t TcpStream::read(void *buffer, size_t count) const {
    return recv(this->sockfd, buffer, count, 0);
}
} // namespace net