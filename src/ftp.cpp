#include "ftp.hpp"
#include <cassert>
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <poll.h>
#include <regex>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using std::string;

static char __STR[1024];
#define _(fmt, ...)                                                            \
    {                                                                          \
        sprintf(__STR, fmt, ##__VA_ARGS__);                                    \
        std::cout << __STR << std::endl;                                       \
    }
#define R(x)                                                                   \
    { std::cout << (x).reply; }

#define PAGE 4069
static void pipe_and_close(int in, int out) {
    void *buf = malloc(PAGE);
    ssize_t ret;
    while (0 != (ret = read(in, buf, PAGE))) {
        if (ret == -1) {
            _("Error: %s", strerror(errno));
            goto RET;
        }
        ret = write(out, buf, ret);
        if (ret == -1) {
            _("Error: %s", strerror(errno));
            goto RET;
        }
    }
RET:
    free(buf);
    close(in);
    close(out);
}
/*
static string get_filename(const string &s) {
    string ret;
    for (auto i = s.rbegin(); i != s.rend() && *i != '/'; i++) {
        ret += *i;
    }
    for (int i = 0, j = ret.length() - 1; i < j; i++, j--) {
        std::swap(ret[i], ret[j]);
    }
    return ret;
}
*/

/*
 * FTP reply starts with a three-digit number indicating reply's code.
 *
 * This function should only be called after `is_final_reply` or
 * `is_half_reply`, because it doesn't perform any check on the input.
 * */
static int get_reply_code(const string &rep) {
    return (rep[0] - 48) * 100 + (rep[1] - 48) * 10 + (rep[2] - 48);
}

static bool is_num(char c) { return c <= '9' && c >= '0'; }

/*
 * For multi-line reply, FTP begins a reply sequence with "AAA-Some text"
 * and ends with "AAA Some optional text".
 *
 * This this implementation, "AAA-" is called a "half reply" and
 * "AAA " is called a "final reply".
 * */
static bool is_final_reply(const string &rep) {
    return rep.length() >= 3 && is_num(rep[0]) && is_num(rep[1]) &&
           is_num(rep[2]) && (rep.length() == 3 || rep[3] == ' ');
}

static bool is_half_reply(const string &rep) {
    return rep.length() >= 4 && is_num(rep[0]) && is_num(rep[1]) &&
           is_num(rep[2]) && rep[3] == '-';
}

/*
 * Reply code 227 has six numbers indication IP address and port,
 * separated by ',':
 * XX,XX,XX,XX,XX,XX
 *
 * This struct and the below function parses 227 reply.
 * */
struct __227Result {
    string ip;
    uint16_t port;
};

/*
 * Parse 227 reply.
 *
 * Crash if the code is not 227.
 * Throw is the code is 227 but doesn't contain valid data.
 *
 * Invalid data means it doesn't contain six numbers separated by ',',
 * or one of these number is not in the range [0, 255].
 * */
static __227Result parse_227_reply(const ftp::Reply &rep) {
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

static bool linux_chdir(const char *path) {
    if (-1 == chdir(path)) {
        auto err = errno;
        _("Error: %s", strerror(err));
        return false;
    }
    return true;
}

/*
 * Here goes some useful macros for implementation.
 * */
// Catch network error, print it then return.
#define __catch_net                                                            \
    catch (const char *e) {                                                    \
        _("Error: %s", e);                                                     \
        return false;                                                          \
    }
// Catch invalid reply of server, print it then return.
#define __catch_rep                                                            \
    catch (Reply r) {                                                          \
        _("Server return invalid reply: \n%s", r.reply.c_str());               \
        return true;                                                           \
    }
// Check the connection
#define __check_connection                                                     \
    {                                                                          \
        if (this->cc == nullptr) {                                             \
            _("Not connected");                                                \
            return false;                                                      \
        }                                                                      \
    }

namespace ftp {

/*
 * Note on try-catch writting:
 *
 * Every public functions of this class must not throw.
 * Error must be handled inside the function.
 * Which means that every lines of code must be wrapped in
 * a try-catch block.
 *
 * Most of the error handling in this code is just printing it out,
 * use `__catch_net` and `__catch_rep` macros if you don't need special
 * logic for error handling.
 *
 * For private functions, if no special error handling is needed,
 * it must not catch the error, let the caller catch it.
 * And if server's reply is invalid, it must be thrown.
 *
 * At this time, only two things are thrown:
 * - `char *`, for network errors
 * - `struct Reply`, for invalid reply from server (only invalid,
 *   negative reply must not thrown).
 *
 * */

/*
 * Note on implementation:
 *
 * For each FTP command sent, there are one or more replies.
 * And for a reply, there can be other replies follow it.
 *
 * All reply sequences and meaning of each reply is documented
 * in RFC 959: https://tools.ietf.org/html/rfc959 .
 * Some replies are documented in 'doc/commands.txt'.
 *
 * Implementation must handle all possible replies for each command.
 *
 * There is the `_()` macro to help on printing formatted string,
 * and `R()` to print `struct Reply`.
 *
 * Sometime, logic for positive and negative reply handling is not different,
 * but they should be handle separately.
 * */
Ftp::Ftp() {}
Ftp::~Ftp() {
    if (this->cc != nullptr) {
        delete this->cc;
    }
    if (this->active_listen != nullptr) {
        delete this->active_listen;
    }
}

/*
 * Connect (or reconnect) to server,
 * then send username and password to login.
 * */
bool Ftp::login(const string &ip, uint16_t port, const string &name,
                const string &passwd) {
    // Connect to the server
    try {
        if (this->cc != nullptr) {
            // TODO should close the connection properly
            delete this->cc;
            this->cc = nullptr;
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
            return false;
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
            return true;
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
            return false;
        default:
            throw rep;
        }
    }
    __catch_net __catch_rep;

    // Send password
    try {
        _("Sending password");
        this->cc->send("PASS " + passwd, false);
        auto rep = this->read_reply();
        switch (rep.code) {
        case 230:
            R(rep);
            return true;
        case 202:
        case 530:
        case 500:
        case 501:
        case 503:
        case 421:
        case 332:
            R(rep);
            _("Login failed");
            return false;
        default:
            throw rep;
        }
    }
    __catch_net __catch_rep;
}

/*
 * Directory listing.
 *
 * First, send `PORT` or `PASV` to setup
 * data connection establishment method.
 *
 * Then, send LIST command, open data connection,
 * and handle replies.
 * */
bool Ftp::list(const string &path) {
    try {
        __check_connection;
        if (!this->port_pasv()) {
            return false;
        }
        this->cc->send(path.empty() ? "LIST" : "LIST " + path);
        auto connector = this->setup_data_connection();
        auto rep = this->read_reply();
        switch (rep.code) {
        case 450:
        case 500:
        case 501:
        case 502:
        case 421:
        case 530:
            R(rep);
            return false;
        case 125:
        case 150: {
            R(rep);
            auto dc = connector.wait();
            std::cout << dc.read_all().str();
            rep = this->read_reply();
            switch (rep.code) {
            case 226:
            case 250:
                R(rep);
                return true;
            case 425:
            case 426:
            case 451:
                R(rep);
                return false;
            default:
                throw rep;
            }
        }
        default:
            throw rep;
        }
    }
    __catch_net __catch_rep;
}

/*
 * Read a reply.
 * The returned struct contains reply's code in number,
 * and the full text recieved.
 *
 * Reply can contain single or multiple lines.
 * */
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

/*
 * Send PORT or PASV to setup control connection establishment method.
 * */
bool Ftp::port_pasv() {
    if (this->active) {
        if (this->active_listen != nullptr)
            return true;
        auto info = this->cc->get_listen_address();
        string cmd = "PORT " + std::to_string(info.data[0]);
        for (size_t i = 1; i < 4; i++) {
            cmd += "," + std::to_string(info.data[i]);
        }

        if (this->active_listen == nullptr) {
            this->active_listen = new net::TcpListener(0);
        }
        info = this->active_listen->get_listen_address();
        for (size_t i = 4; i < 6; i++) {
            cmd += "," + std::to_string(info.data[i]);
        }
        this->cc->send(cmd);
        auto rep = this->read_reply();
        switch (rep.code) {
        case 200:
            R(rep);
            return true;
        case 500:
        case 501:
        case 421:
        case 530:
            R(rep);
            return false;
        default:
            throw rep;
        }
    } else {
        this->cc->send("PASV");
        auto rep = this->read_reply();
        switch (rep.code) {
        case 227: {
            R(rep);
            auto ret = parse_227_reply(rep);
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
}

bool Ftp::mkdir(const string &path) {
    try {
        __check_connection;
        this->cc->send("MKD " + path);
        auto rep = this->read_reply();
        switch (rep.code) {
        case 257:
            R(rep);
            return true;
        case 500:
        case 501:
        case 502:
        case 421:
        case 530:
        case 550:
            R(rep);
            return false;
        default:
            throw rep;
        }
    }
    __catch_net __catch_rep;
}

bool Ftp::rmdir(const string &path) {
    try {
        __check_connection;
        this->cc->send("RMD " + path);
        auto rep = this->read_reply();
        switch (rep.code) {
        case 250:
            R(rep);
            return true;
        case 500:
        case 501:
        case 502:
        case 421:
        case 530:
        case 550:
            R(rep);
            return false;
        default:
            throw rep;
        }
    }
    __catch_net __catch_rep;
}

/*
 * Change current directory on server.
 *
 * If path is "..", use CDUP command to make sure it works when
 * remote system's syntax for parent directory is not "..".
 *
 * In RFC 959, CDUP returns 200, but some implementations use 250.
 * */
bool Ftp::chdir(const string &path) {
    try {
        __check_connection;
        if (path == "..") {
            this->cc->send("CDUP");
            auto rep = this->read_reply();
            switch (rep.code) {
            case 200:
            case 250:
                R(rep);
                return true;
            case 500:
            case 501:
            case 502:
            case 421:
            case 530:
            case 550:
                R(rep);
                return false;
            default:
                throw rep;
            }
        } else {
            this->cc->send("CWD " + path);
            auto rep = this->read_reply();
            switch (rep.code) {
            case 250:
                R(rep);
                return true;
            case 500:
            case 501:
            case 502:
            case 421:
            case 530:
            case 550:
                R(rep);
                return false;
            default:
                throw rep;
            }
        }
    }
    __catch_net __catch_rep;
}

bool Ftp::pwd() {
    try {
        __check_connection;
        this->cc->send("PWD");
        auto rep = this->read_reply();
        switch (rep.code) {
        case 257:
            R(rep);
            return true;
        case 500:
        case 501:
        case 502:
        case 421:
        case 550:
            R(rep);
            return false;
        default:
            throw rep;
        }
    }
    __catch_net __catch_rep;
}

bool Ftp::local_chdir(const string &path) { return linux_chdir(path.c_str()); }

bool Ftp::local_pwd() {
    char *str = get_current_dir_name();
    if (str == nullptr) {
        auto err = errno;
        _("Error: %s", strerror(err));
        return false;
    }
    _("%s", str);
    free(str);
    return true;
}

bool Ftp::set_active() {
    this->active = true;
    _("Set mode to active");
    return true;
}

bool Ftp::set_passive() {
    this->active = false;
    _("Set mode to passive");
    return true;
}

bool Ftp::remove(const string &path) {
    try {
        __check_connection;
        this->cc->send("DELE " + path);
        auto rep = this->read_reply();
        switch (rep.code) {
        case 250:
            R(rep);
            return true;
        case 450:
        case 550:
        case 500:
        case 501:
        case 502:
        case 421:
        case 530:
            R(rep);
            return false;
        default:
            throw rep;
        }
    }
    __catch_net __catch_rep;
}

bool Ftp::quit() {
    try {
        __check_connection;
        this->cc->send("QUIT");
        auto rep = this->read_reply();
        switch (rep.code) {
        case 221:
            R(rep);
            return true;
        case 500:
            R(rep);
            return false;
        default:
            throw rep;
        }
    }
    __catch_net __catch_rep;
}

Connector::Connector(net::TcpListener *_listen) {
    this->active = 1;
    this->listen = _listen;
}
Connector::Connector(const string &host, uint16_t port) {
    this->active = 0;
    this->stream = new net::TcpStream(host.c_str(), port, true);
}
Connector::~Connector() {
    if (this->listen != nullptr)
        delete this->listen;
    if (this->stream != nullptr)
        delete this->stream;
}
net::TcpStream Connector::wait() {
    if (this->active) {
        return this->listen->next();
    } else {
        struct pollfd fd;
        fd.fd = this->stream->fd();
        fd.events = POLLOUT;
        poll(&fd, 1, -1);
        if (fd.revents != POLLOUT) {
            throw "Poll return unexpected event";
        } else {
            net::TcpStream res = *this->stream;
            delete this->stream;
            this->stream = nullptr;
            res.set_block();
            return res;
        }
    }
}

Connector Ftp::setup_data_connection() {
    if (this->active) {
        auto tmp = this->active_listen;
        this->active_listen = nullptr;
        return Connector(tmp);
    } else {
        return Connector(this->dc_param.ip, this->dc_param.port);
    }
}

bool Ftp::store(const string &local_path, const string &remote_path) {
    try {
        __check_connection;
        auto file = open(local_path.c_str(), O_RDONLY);
        if (file == -1) {
            throw strerror(errno);
        }
        if (!this->port_pasv()) {
            return false;
        }

        this->cc->send("STOR " + remote_path);
        auto connector = this->setup_data_connection();
        auto rep = this->read_reply();
        switch (rep.code) {
        case 532:
        case 450:
        case 452:
        case 553:
        case 500:
        case 501:
        case 421:
        case 530:
            R(rep);
            return false;
        case 125:
        case 150: {
            R(rep);
            auto dc = connector.wait();
            pipe_and_close(file, dc.fd());
            auto rep = this->read_reply();
            switch (rep.code) {
            case 226:
            case 250:
                R(rep);
                return true;
            case 110:
            case 425:
            case 426:
            case 451:
            case 551:
            case 552:
                R(rep);
                return false;
            default:
                throw rep;
            }
        }
        default:
            throw rep;
        }
    }
    __catch_net __catch_rep;
}

bool Ftp::retrieve(const string &local_path, const string &remote_path) {
    try {
        __check_connection;
        if (!this->port_pasv()) {
            return false;
        }
        this->cc->send("RETR " + remote_path);
        auto connector = this->setup_data_connection();
        auto rep = read_reply();
        switch (rep.code) {
        case 450:
        case 550:
        case 500:
        case 501:
        case 421:
        case 530:
            R(rep);
            return false;
        case 125:
        case 150: {
            R(rep);
            auto dc = connector.wait();
            auto file = fileno(fopen(local_path.c_str(), "w"));
            if (file == -1) {
                throw strerror(errno);
            }
            pipe_and_close(dc.fd(), file);
            auto rep = this->read_reply();
            switch (rep.code) {
            case 226:
            case 250:
                R(rep);
                return true;
            case 425:
            case 426:
            case 451:
                R(rep);
                return false;
            default:
                throw rep;
            }
        }
        default:
            throw rep;
        }
    }
    __catch_net __catch_rep;
}

}; // namespace ftp
