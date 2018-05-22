#include "main.hpp"
#include "net.hpp"
#include <iostream>
#include <thread>

int main() {
    try {
        command cmd;
        cmd = readCommand();
        
        if (cmd.type == commandType::LOGIN){
            login *lg = (login *)cmd.value;
            std::cout << lg->ip << std::endl;
            std::cout << lg->userName << std::endl;
        }
        else if (cmd.type == commandType::LIST_FILE){
            fileCommand* ls = (fileCommand*)cmd.value;
            std::cout << ls->remote << std::endl;
            std::cout << ls->localFile << std::endl;
        } 

        delete cmd.value;
    } catch (const char *e) {
        std::cout << e << std::endl;
    }
    return 0;
}