#ifndef _FTP_H_
#define _FTP_H_

#include "net.hpp"

namespace ftp {

struct Reply {
    int code;
    std::string reply;
};
class Connector {
    int active = -1;
    net::TcpListener *listen = nullptr;
    net::TcpStream *stream = nullptr;

public:
    Connector(net::TcpListener *_listen);
    Connector(const std::string &host, uint16_t port);
    ~Connector();
    net::TcpStream wait();
};

class Ftp {

private:
    net::Messenger *cc = nullptr;
    bool active = false;

    struct {
        std::string ip;
        uint16_t port = 0;
    } dc_param;
    net::TcpListener *active_listen = nullptr;

    Reply read_reply();

    bool port_pasv();
    Connector setup_data_connection();

public:
    ~Ftp();
    Ftp();
    bool login(const std::string &ip, uint16_t port, const std::string &name,
               const std::string &passwd);
    bool list(const std::string &path = "");
    bool mkdir(const std::string &path);
    bool rmdir(const std::string &path);
    bool chdir(const std::string &path);
    bool pwd();
    bool local_chdir(const std::string &path);
    bool local_pwd();
    bool set_active();
    bool set_passive();
    bool remove(const std::string &path);
    bool quit();
    bool store(const std::string &local_path, const std::string &remote_path);
    bool retrieve(const std::string &local_path,
                  const std::string &remote_path);
};
} // namespace ftp
#endif