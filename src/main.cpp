#include "ftp.hpp"
#include "main.hpp"
#include "net.hpp"
#include <iostream>
#include <thread>
using std::cout;
using std::endl;

int main() {
    ftp::Ftp f;
    f.login("127.0.0.1", 21, "thiennu", "366117187");
    while (true){
        command cmd;
        cmd = readCommand();
        cout << (int)cmd.type << endl;

        if (cmd.type == commandType::LOGIN){
            login *lg = (login *)cmd.value;
            f.login(lg->ip, lg->port, lg->userName, lg->password);
            delete lg;
        }
        else if (cmd.type == commandType::LIST_FILE){
            fileCommand* ls = (fileCommand*)cmd.value;
            //cout << ls->remote << endl;
            f.list(ls->remote);
            delete ls;
        } 
        else if (cmd.type == commandType::PUT){
            std::string* path = (std::string*)cmd.value;
            f.store(*path);
        }
        else if (cmd.type == commandType::GET){
            std::string* path = (std::string*)cmd.value;
            f.retrieve(*path);
        }
        else if (cmd.type == commandType::MPUT){
            dirList* ld = (dirList*)cmd.value;
            for (int i = 0; i < ld->numDir; i++){
                f.store(ld->arrDir[i]);
            }
        }
        else if (cmd.type == commandType::MGET){
            dirList* ld = (dirList*)cmd.value;
            for (int i = 0; i < ld->numDir; i++){
                f.retrieve(ld->arrDir[i]);
            }
        }
        else if (cmd.type == commandType::PWD){
            f.pwd();
        }

    } 
    return 0;
}
