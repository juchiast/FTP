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

public:
    Messenger(const TcpStream&);
    ~Messenger();
    int len_first_cmd();
    void send(const void* buffer, int count);
    std::string receive();
};
} // namespace net
#endif