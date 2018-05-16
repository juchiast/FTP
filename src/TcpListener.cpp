#include "net.hpp"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * This function return a `struct sockaddr_in` that presents
 * a local socket with port `local_port`.
 *
 * If port is 0, calling `bind` on this address will allow
 * binding to a random port.
 * */
static struct sockaddr_in __get_address(uint16_t local_port) {
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(local_port);
    addr.sin_family = AF_INET;
    memset(addr.sin_zero, 0, sizeof(addr.sin_zero));
    return addr;
}

/*
 * Create a new socket and bind it to the address in `addr`.
 * This function return a file descriptor of the socket.
 *
 * Also set SO_REUSEADDR to allow reusing port immediately
 * after releasing it (by default, address will not be available
 * for some time after releasing).
 *
 * This function throws if there is any error.
 * */
static int __bind(struct sockaddr_in addr) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        throw strerror(errno);
    }
    int temp = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(int)) == -1) {
        throw strerror(errno);
    }
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        throw strerror(errno);
    }
    return sock;
}

static void __close(int fd) {
    close(fd);
}

namespace net {
TcpListener::~TcpListener() {
    this->close();
}

/*
 * Bind and listen for connections on port `local_port`.
 *
 * If port is 0, listen on a random available port.
 *
 * A listening socket is not able of sending and receiving.
 * Only a connection (a.k.a `TcpStream`) is able to do so.
 * */
TcpListener::TcpListener(uint16_t local_port) {
    this->listenfd = __bind(__get_address(local_port));
    if (listen(this->listenfd, 10000) == -1) {
        throw strerror(errno);
    }
}

/*
 * Return a new connection to this socket.
 * This function blocks until there is a connection.
 * */
TcpStream TcpListener::next() {
    int sockfd = accept(this->listenfd, nullptr, nullptr);
    if (sockfd < 0) {
        throw strerror(errno);
    }
    return TcpStream(sockfd);
}

/*
 * Use `dup` to create a duplicate of this socket.
 *
 * See `TcpStream` document of this function for more.
 * */
TcpListener &TcpListener::operator=(const TcpListener &t) {
    this->close();
    this->listenfd = dup(t.listenfd);
    if (this->listenfd == -1) {
        throw strerror(errno);
    }
    return *this;
}

/*
 * Use `dup` to create a duplicate of this socket.
 *
 * See `TcpStream` document of this function for more.
 * */
TcpListener::TcpListener(const TcpListener &t) {
    this->listenfd = dup(t.listenfd);
    if (this->listenfd == -1) {
        throw strerror(errno);
    }
}

/*
 * Just close, there is no `shutdown` for this kind of socket.
 * */
void TcpListener::close() {
    __close(this->listenfd);
}

TcpListener::TcpListener() {}
} // namespace net
