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

    Reply read_reply();

public:
    ~Ftp();
    Ftp();
    void login(const std::string &ip, uint16_t port, const std::string &name,
               const std::string &passwd);
    void list(const std::string &path = "");
};
};

#endif