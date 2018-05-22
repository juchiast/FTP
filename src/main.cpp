#include "main.hpp"
#include "ftp.hpp"
#include "net.hpp"
#include <iostream>
#include <thread>
using std::cout;
using std::endl;

int main() {
    ftp::Ftp f;
    f.login("127.0.0.1", 21, "thiennu", "366117187");
    while (true) {
        command cmd;
        cmd = readCommand();

        if (cmd.type == commandType::LOGIN) {
            login *lg = (login *)cmd.value;
            f.login(lg->ip, lg->port, lg->userName, lg->password);
            delete lg;
        } 
        else if (cmd.type == commandType::LIST_FILE) {
            fileCommand *ls = (fileCommand *)cmd.value;
            f.list(ls->remote);
            delete ls;
        } 
        else if (cmd.type == commandType::PUT) {
            std::string *path = (std::string *)cmd.value;
            f.store(*path);
        } 
        else if (cmd.type == commandType::GET) {
            std::string *path = (std::string *)cmd.value;
            cout << *path << endl;
            f.retrieve(*path);
        } 
        else if (cmd.type == commandType::MPUT) {
            dirList *ld = (dirList *)cmd.value;
            for (int i = 0; i < ld->numDir; i++) {
                f.store(ld->arrDir[i]);
            }
        } 
        else if (cmd.type == commandType::MGET) {
            dirList *ld = (dirList *)cmd.value;
            for (int i = 0; i < ld->numDir; i++) {
                f.retrieve(ld->arrDir[i]);
            }
        } 
        else if (cmd.type == commandType::CD) {
            std::string *path = (std::string *)cmd.value;
            f.chdir(*path);
        } 
        else if (cmd.type == commandType::LCD) {
            std::string *path = (std::string *)cmd.value;
            f.local_chdir(*path);
        } 
        else if (cmd.type == commandType::DELETE) {
            std::string *path = (std::string *)cmd.value;
            f.remove(*path);
        } 
        else if (cmd.type == commandType::MDELETE) {
            dirList *ld = (dirList *)cmd.value;
            for (int i = 0; i < ld->numDir; i++) {
                f.remove(ld->arrDir[i]);
            }
        } 
        else if (cmd.type == commandType::MKDIR) {
            std::string *path = (std::string *)cmd.value;
            f.mkdir(*path);
        } 
        else if (cmd.type == commandType::RMKDIR) {
            std::string *path = (std::string *)cmd.value;
            f.rmdir(*path);
        } 
        else if (cmd.type == commandType::PWD) {
            f.pwd();
        } 
        else if (cmd.type == commandType::EXIT) {
            f.quit();
        } 
        else if (cmd.type == commandType::PASSIVE) {
            f.set_passive();
        } 
        else if (cmd.type == commandType::ACTIVE) {
            f.set_active();
        }
    }
    return 0;
}
