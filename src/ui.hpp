#ifndef _UI_H_
#define _UI_H_
#include "ftp.hpp"
#include <string>

namespace ui{

const int maxFile = 100;

// Cac cau truc yeu cau
enum class commandType {
    ERROR,
    LOGIN,
    HELP,
    LIST_FILE,
    PUT,
    GET,
    MPUT,
    MGET,
    CD,
    LCD,
    DELETE,
    MDELETE,
    MKDIR,
    RMKDIR,
    PWD,
    PASSIVE,
    ACTIVE,
    EXIT,
};

// Cau truc login
struct login {
    std::string ip = "";
    uint16_t port = 21;
    std::string userName = "";
    std::string password = "";
};

struct fileCommand {
    std::string remote = "";
    std::string localFile = "";
};

struct dirList {
    std::string arrDir[maxFile];
    int numDir = 0;
};

// Kieu du dieu command
struct command {
    enum commandType type;
    void *value = NULL;
};
int run(void *ftp);
}
#endif
