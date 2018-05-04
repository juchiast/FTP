#ifndef _NET_HPP_
#define _NET_HPP_
#include <stdint.h>

namespace net {
class TcpStream {
private:
    int sockfd;
    TcpStream();

public:
    ~TcpStream();
    static TcpStream connect(const char *remote_ip, uint16_t remote_port);
    size_t write(void *buffer, size_t count);
    size_t read(void *buffer, size_t count);
    void close_read();
    void close_write();
    void close_both();
};
} // namespace net
#endif