#include "ftp.hpp"
#include <cassert>
#include <iostream>
#include <regex>

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

struct __227Result {
    string ip;
    uint16_t port;
};

static __227Result __parse_227_reply(const ftp::Reply &rep) {
    assert(rep.code == 227);
    static const std::regex re(R"((\d+),(\d+),(\d+),(\d+),(\d+),(\d+))");
    std::smatch sm;
    if (std::regex_search(rep.reply, sm, re)) {
        assert(sm.size() == 7);
        int nums[6];
        for (size_t i = 1; i <= 6; i++) {
            int x = std::atoi(sm[i].str().c_str());
            if (x < 0 || x > 255) {
                throw rep;
            }
            nums[i - 1] = x;
        }
        __227Result ret;
        char buf[16];
        sprintf(buf, "%d.%d.%d.%d", nums[0], nums[1], nums[2], nums[3]);
        ret.ip = buf;
        ret.port = (nums[4] << 8) | nums[5];
        return ret;
    } else {
        throw rep;
    }
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

    // Connect to the server
    try {
        if (this->cc != NULL) {
            delete this->cc;
        }
        _("Connecting to server %s:%d", ip.c_str(), port);
        this->cc = new net::Messenger(net::TcpStream(ip.c_str(), port));
        auto rep = this->read_reply();
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
        } else {
            throw rep;
        }
    }
    __catch_net __catch_rep;

    // Send username
    try {
        _("Sending username");
        this->cc->send("USER " + name);
        auto rep = this->read_reply();
        switch (rep.code) {
        case 230:
            R(rep);
            return;
        case 331:
            R(rep);
            break;
        case 530:
        case 500:
        case 501:
        case 421:
        case 332:
            R(rep);
            _("Login failed")
            return;
        default:
            throw rep;
        }
    }
    __catch_net __catch_rep;

    // Send password
    try {
        _("Sending password");
        this->cc->send("PASS " + passwd);
        auto rep = this->read_reply();
        switch (rep.code) {
        case 230:
            R(rep);
            return;
        case 202:
        case 530:
        case 500:
        case 501:
        case 503:
        case 421:
        case 332:
            R(rep);
            _("Login failed");
            return;
        default:
            throw rep;
        }
    }
    __catch_net __catch_rep;
}

void Ftp::list(const string &path) {
    try {
        if (!this->port_pasv()) {
            return;
        }
        this->cc->send(path.empty() ? "LIST" : "LIST " + path);
        auto conn = this->setup_data_connection();
        auto rep = this->read_reply();
        switch (rep.code) {
        case 450:
        case 500:
        case 501:
        case 502:
        case 421:
        case 530:
            R(rep);
            return;
        case 125:
        case 150: {
            R(rep);
            char *buf = new char[1024];
            auto size = conn.read(buf, 1024);
            buf[size] = 0;
            std::string s = buf;
            delete[] buf;
            rep = this->read_reply();
            switch (rep.code) {
            case 226:
            case 250:
                R(rep);
                _("%s", s.c_str());
                return;
            case 425:
            case 426:
            case 451:
                R(rep);
                return;
            default:
                throw rep;
            }
            return;
        }
        default:
            throw rep;
        }
    }
    __catch_net __catch_rep;
}

net::TcpStream Ftp::setup_data_connection() {
    if (this->active) {
        throw "Not implemented";
    }

    return net::TcpStream(this->dc_param.ip.c_str(), this->dc_param.port);
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

bool Ftp::port_pasv() {
    if (this->active) {
        throw "Not implemented";
    }
    this->cc->send("PASV");
    auto rep = this->read_reply();
    switch (rep.code) {
    case 227: {
        R(rep);
        auto ret = __parse_227_reply(rep);
        this->dc_param.ip = ret.ip;
        this->dc_param.port = ret.port;
        return true;
    }
    case 500:
    case 501:
    case 502:
    case 421:
    case 530:
        R(rep);
        this->dc_param.ip = "";
        this->dc_param.port = 0;
        return false;
    default:
        throw rep;
    }
}

}; // namespace ftp