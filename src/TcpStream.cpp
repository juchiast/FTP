#include "net.hpp"
#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
using namespace net;

/*
 * Return a `struct sockaddr_in` to use in later syscall by combining
 * IP address and port.
 *
 * `ip` is a string present an IPv4 address
 * `port` is the port number to be use
 *
 * This function throw if there is any error.
 * */
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

/*
 * Connect to the address provided by `addr`
 *
 * This function return a file descriptor that point to
 * the opened connection (everything is a file in Linux).
 *
 * This function throw if there is any error.
 * */
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

/*
 * This is just a wrapper to the `__connect` function above.
 * */
TcpStream::TcpStream(const char *remote_ip, uint16_t remote_port) {
    this->sockfd = __connect(__get_address(remote_ip, remote_port));
}

/*
 * Read at most `count` bytes into `buffer`.
 * Throw if there is error.
 * Return number of bytes read.
 * */
size_t TcpStream::read(void *buffer, size_t count) {
    auto r = recv(this->sockfd, buffer, count, 0);
    if (r < 0) {
        throw strerror(errno);
    }
    return r;
}

/*
 * Send `count` bytes in `buffer`.
 * Throw if there is error.
 * Return number of bytes sent.
 * */
size_t TcpStream::write(const void *buffer, size_t count) {
    auto r = send(this->sockfd, buffer, count, 0);
    if (r < 0) {
        throw strerror(errno);
    }
    return r;
}

/*
 * This function close the read-gate, which means we cannot read
 * from this socket anymore.
 *
 * This function affects the underlying connection, not just this
 * file descriptor.
 *
 * This function throw if there is error and not ENOTCONN error,
 * since there is no benefit in catching error closing an already
 * closed connection.
 * */
void TcpStream::close_read() {
    if (shutdown(this->sockfd, SHUT_RD) == -1 && errno != ENOTCONN) {
        throw strerror(errno);
    }
}

/*
 * Close the write-gate, which means we cannot write to this socket
 * after this call.
 *
 * Also has some notes as in the `close_read` function.
 * */
void TcpStream::close_write() {
    if (shutdown(this->sockfd, SHUT_WR) == -1 && errno != ENOTCONN) {
        throw strerror(errno);
    }
}

/*
 * Close both read and write gate.
 * */
void TcpStream::close_both() {
    if (shutdown(this->sockfd, SHUT_RDWR) == -1 && errno != ENOTCONN) {
        throw strerror(errno);
    }
}

/*
 * We don't use `shutdown` here because many file descriptor can
 * point to the same underlying connection.
 * Using `shutdown` will cause other copies of this file descriptor to
 * be closed as well.
 *
 * Also, because this is a destructor, don't check for error and
 * throw here.
 * */
TcpStream::~TcpStream() { close(this->sockfd); }

/*
 * Just copy value of `_sockfd` into this new instance.
 * This constructor acts like move constructor. 
 * */
TcpStream::TcpStream(int _sockfd) { this->sockfd = _sockfd; }

/*
 * Make a copy of `t` using `dup` syscall.
 *
 * If many copies of a connection is made, the connection closes
 * when all its file descriptor is closed, or when we call `shutdown`
 * on any of its file descriptor.
 * */
TcpStream &TcpStream::operator=(const TcpStream &t) {
    close(this->sockfd);
    this->sockfd = dup(t.sockfd);
    if (this->sockfd == -1) {
        throw strerror(errno);
    }
    return *this;
}

/*
 * Same meaning as `operator=`.
 * */
TcpStream::TcpStream(const TcpStream &t) {
    this->sockfd = dup(t.sockfd);
    if (this->sockfd == -1) {
        throw strerror(errno);
    }
}

#define BUF_SIZE 4069

/*
 * Read until the other end of this socket close the connection.
 *
 * This function return a type `ReadAll`, which is a wrapper of
 * `vector<uint8_t>` with some heplper functions.
 * */
ReadAll TcpStream::read_all() {
    ReadAll ret;
    std::vector<uint8_t> buffer(BUF_SIZE);
    size_t size;
    while (0 != (size = this->read(buffer.data(), BUF_SIZE))) {
        ret.data.insert(ret.data.end(), buffer.begin(), buffer.begin() + size);
    }
    return ret;
}

/*
 * Convert the inner `vector<uint8_t>` to a string.
 *
 * Since strings don't contain '\0', we stop when '\0' is found.
 * */
std::string ReadAll::str() const {
    std::string ret;
    for (uint8_t c: this->data) {
        if (c == 0) {
            break;
        }
        ret += c;
    }
    return ret;
}
} // namespace net
