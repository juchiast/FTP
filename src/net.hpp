#ifndef _NET_HPP_
#define _NET_HPP_
#include <stdint.h>
#include <sys/socket.h>

namespace net {
class TcpStream {
    int sockfd;
public:
    TcpStream() = delete;
    TcpStream(const char *remote_ip, uint16_t remote_port);
    void write(void *buffer, size_t count) const;
    size_t read(void *buffer, size_t count) const;
};
} // namespace net
#endif