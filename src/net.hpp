#ifndef _NET_HPP_
#define _NET_HPP_
#include <stdint.h>
#include <stdlib.h>
#include <string>

namespace net {

class TcpStream {
private:
    int sockfd;
    TcpStream();

public:
    ~TcpStream();
    TcpStream(const char *remote_ip, uint16_t remote_port);
    TcpStream(int sockfd);
    size_t write(const void *buffer, size_t count);
    size_t read(void *buffer, size_t count);
    void close_read();
    void close_write();
    void close_both();

    TcpStream &operator=(const TcpStream &);
    TcpStream(const TcpStream &);
};

class TcpListener {
private:
    int listenfd;
    TcpListener();

public:
    ~TcpListener();
    static TcpListener bind(uint16_t local_port);
    TcpStream next();

    TcpListener(const TcpListener &);
    TcpListener &operator=(const TcpListener &);
    void close();
};

class Messenger {
private:
    TcpStream stream;
    std::string buff;

    int len_first_cmd();

public:
    Messenger(const TcpStream &);
    ~Messenger();
    void send(const std::string &);
    std::string receive();
};

} // namespace net
#endif