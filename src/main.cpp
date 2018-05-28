#include "ui.hpp"
#include "ftp.hpp"
#include "net.hpp"
#include <iostream>
#include <thread>
using std::cout;
using std::endl;

int main() {
    ftp::Ftp f;
    while (true) {
        command cmd;
        cmd = readCommand();

        switch (cmd.type) {
        case commandType::LOGIN: {
            login *lg = (login *)cmd.value;
            f.login(lg->ip, lg->port, lg->userName, lg->password);
            delete lg;
            break;
        }
        case commandType::LIST_FILE: {
            fileCommand *ls = (fileCommand *)cmd.value;
            f.list(ls->remote);
            delete ls;
            break;
        }
        case commandType::PUT: {
            fileCommand *path = (fileCommand *)cmd.value;
            f.store(path->localFile, path->remote);
            delete path;
            break;
        }
        case commandType::GET: {
            fileCommand *path = (fileCommand *)cmd.value;
            f.retrieve(path->localFile, path->remote);
            delete path;
            break;
        }
        case commandType::MPUT: {
        }
        case commandType::MGET: {
        }
        case commandType::CD: {
            std::string *path = (std::string *)cmd.value;
            f.chdir(*path);
            delete path;
            break;
        }
        case commandType::LCD: {
            std::string *path = (std::string *)cmd.value;
            f.local_chdir(*path);
            delete path;
            break;
        }
        case commandType::DELETE: {
            std::string *path = (std::string *)cmd.value;
            f.remove(*path);
            delete path;
            break;
        }
        case commandType::MDELETE: {
            dirList *ld = (dirList *)cmd.value;
            for (int i = 0; i < ld->numDir; i++) {
                f.remove(ld->arrDir[i]);
            }
            delete ld;
            break;
        }
        case commandType::MKDIR: {
            std::string *path = (std::string *)cmd.value;
            f.mkdir(*path);
            delete path;
            break;
        }
        case commandType::RMKDIR: {
            std::string *path = (std::string *)cmd.value;
            f.rmdir(*path);
            delete path;
            break;
        }
        case commandType::PWD:
            f.pwd();
            break;
        case commandType::EXIT:
            f.quit();
            return 0;
        case commandType::PASSIVE:
            f.set_passive();
            break;
        case commandType::ACTIVE:
            f.set_active();
            break;

        default:
            cout << "?Invalid command" << endl;
        }
    }
    return 0;
}
