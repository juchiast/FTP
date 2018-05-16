#include "ftp.hpp"
#include <iostream>

using std::string;

static char __STR[1024];
#define _(fmt, ...)                                                            \
    {                                                                          \
        sprintf(__STR, fmt, ##__VA_ARGS__);                                    \
        std::cout << __STR << std::endl;                                       \
    }

#define R(x)                                                                   \
    { std::cout << (x).reply; }

static int get_reply_code(const string &rep) {
    return (rep[0] - 48) * 100 + (rep[1] - 48) * 10 + (rep[2] - 48);
}

static bool is_num(char c) { return c <= '9' && c >= '0'; }

static bool is_final_reply(const string &rep) {
    return rep.length() >= 3 && is_num(rep[0]) && is_num(rep[1]) &&
           is_num(rep[2]) && (rep.length() == 3 || rep[3] == ' ');
}

static bool is_half_reply(const string &rep) {
    return rep.length() >= 4 && is_num(rep[0]) && is_num(rep[1]) &&
           is_num(rep[2]) && rep[3] == '-';
}

#define __catch_net                                                            \
    catch (char *e) {                                                          \
        _("Error: %s", e);                                                     \
        return;                                                                \
    }
#define __catch_rep                                                            \
    catch (Reply r) {                                                          \
        _("Server return invalid reply: \n%s", r.reply.c_str());               \
        return;                                                                \
    }

namespace ftp {

Ftp::Ftp() {}
Ftp::~Ftp() {
    if (this->cc != NULL) {
        delete this->cc;
    }
}

void Ftp::login(const string &ip, uint16_t port, const string &name,
                const string &passwd) {
    try {
        _("Connecting to server %s:%d", ip.c_str(), port);
        this->cc = new net::Messenger(net::TcpStream(ip.c_str(), port));
        auto rep = read_reply();
        if (rep.code == 120) {
            R(rep);
            rep = read_reply();
        }
        if (rep.code == 220) {
            _("Connected to server");
            R(rep);
        } else if (rep.code == 421) {
            R(rep);
            return;
        }
    }
    __catch_net __catch_rep
}

Reply Ftp::read_reply() {
    Reply rep;
    rep.code = 0;
    while (1) {
        string str = this->cc->receive();
        rep.reply += str;
        rep.reply.push_back('\n');
        if (is_final_reply(str)) {
            int code = get_reply_code(str);
            if (rep.code != 0 && rep.code != code) {
                rep.code = 0;
                throw rep;
            }
            rep.code = code;
            return rep;
        } else if (is_half_reply(str)) {
            int code = get_reply_code(str);
            if (rep.code != 0 && rep.code != code) {
                rep.code = 0;
                throw rep;
            }
            rep.code = code;
        } else if (rep.code == 0) {
            throw rep;
        }
    }
}

}; // namespace ftp
