#ifndef _FTP_H_
#define _FTP_H_

#include "net.hpp"

namespace ftp {

struct Reply {
    int code;
    std::string reply;
};

class Ftp {

private:
    net::Messenger *cc = NULL;
    bool active = false;

    struct {
        std::string ip;
        uint16_t port = 0;
    } dc_param;

    Reply read_reply();

    bool port_pasv();
    net::TcpStream setup_data_connection();

public:
    ~Ftp();
    Ftp();
    void login(const std::string &ip, uint16_t port, const std::string &name,
               const std::string &passwd);
    void list(const std::string &path = "");
};
} // namespace ftp
#endif