#ifndef _NET_HPP_
#define _NET_HPP_
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <vector>

// TODO remove default constructor

namespace net {

struct ReadAll {
    std::vector<uint8_t> data;

    std::string str() const;
};
struct Address {
    uint8_t data[6];
};

class TcpStream {
private:
    int sockfd;
    TcpStream();

public:
    ~TcpStream();
    TcpStream(const char *remote_ip, uint16_t remote_port, bool nonblock = false);
    TcpStream(int sockfd);
    TcpStream(const TcpStream &);
    TcpStream &operator=(const TcpStream &);

    size_t write(const void *buffer, size_t count);
    size_t read(void *buffer, size_t count);
    ReadAll read_all();
    void close_read();
    void close_write();
    void close_both();
    int fd();
    Address get_listen_address();
    void set_block();
};

class TcpListener {
private:
    int listenfd;
    TcpListener();

public:
    ~TcpListener();
    TcpListener(uint16_t local_port);
    TcpStream next();

    TcpListener(const TcpListener &);
    TcpListener &operator=(const TcpListener &);
    void close();
    Address get_listen_address();
};

class Messenger {
private:
    TcpStream stream;
    std::string buff;

    int len_first_cmd();

public:
    Messenger(const TcpStream &);
    ~Messenger();
    void send(const std::string &, bool verbose = true);
    std::string receive();
    Address get_listen_address();
};

} // namespace net
#endif
