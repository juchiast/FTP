#ifndef _NET_HPP_
#define _NET_HPP_
#include <stdint.h>
#include <stdlib.h>

namespace net {

class TcpStream {
private:
    int sockfd;
    TcpStream();

public:
    ~TcpStream();
    static TcpStream connect(const char *remote_ip, uint16_t remote_port);
    static TcpStream from_raw(int sockfd);
    size_t write(void *buffer, size_t count);
    size_t read(void *buffer, size_t count);
    void close_read();
    void close_write();
    void close_both();

    TcpStream& operator=(const TcpStream &);
    TcpStream(const TcpStream &);
};

class TcpListener {
private:
    int listenfd;

public:
    ~TcpListener();
    static TcpListener bind(uint16_t local_port);
    TcpStream next();
};
} // namespace net
#endif