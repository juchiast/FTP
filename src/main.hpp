#ifndef _MAIN_H_
#define _MAIN_H_
#include <stdio.h>
#include "net.hpp"
#include <readline/history.h>
#include <readline/readline.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <map>
#include <unistd.h>
#include <vector>

const int maxFile = 100;

//Cac cau truc yeu cau
enum class commandType{
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
    EXIT
};

//Menu cua cac lenh
std::map<const std::string, const enum commandType> commandMenu = {
    { "login", commandType::LOGIN },
    { "help", commandType::HELP},
    {"ls", commandType::LIST_FILE},
    {"dir", commandType::LIST_FILE},
    {"put", commandType::PUT},
    {"mput", commandType::MPUT},
    {"get", commandType::GET},
    {"mget", commandType::MGET},
    {"cd", commandType::CD},
    {"lcd", commandType::LCD},
    {"delete", commandType::DELETE},
    {"mdelete", commandType::MDELETE},
    {"mkdir", commandType::MKDIR},
    {"rmkdir", commandType::RMKDIR},
    {"pwd", commandType::PWD},
    {"passive", commandType::PASSIVE},
    {"quit", commandType::EXIT},
    {"exit", commandType::EXIT}
};

//Cau truc login
struct login{
    std::string ip = "";
    uint16_t port = 21;
    std::string userName = "";
    std::string password = "";
};

//Kieu du dieu command
struct command{
    enum commandType type;
    void* value = NULL;
};

std::string myReadline(const char* prompt){
    char* tmp = readline(prompt);
    std::string str;
    if (tmp) 
        str.assign(tmp);
    free(tmp);
    return str;
}

login* inputLogin(const std::string str){
    login* info = new login();
    char* tmp;

    if (str != ""){
        info->ip = str;    
    } else {
        info->ip = myReadline("(to) ");
    }

    info->userName = myReadline("Name: ");

    tmp = getpass("Pass: ");
    if (tmp) info->password = getpass("Pass: ");

    return info;
}

std::vector<std::string> readDir(const std::string str){
    std::vector<std::string> dirFiles;

    int spacePos;
    int currPos = 0;
    int i = 0;
    int prevPos = 0;

    do{
        spacePos = str.find(" ",currPos);

        if(spacePos >= 0){
            currPos = spacePos;
            dirFiles.push_back(str.substr(prevPos, currPos - prevPos));
            currPos++;
            prevPos = currPos;
        }

    } while( spacePos >= 0);
    
    for (int i = 0; i < dirFiles.size(); i++)
        std::cout << dirFiles[i] << std::endl;
    return dirFiles;
}

command readCommand(){
    std::string input_string;
    input_string = myReadline("ftp>");
//Lay yeu cau
    int pos = input_string.find(" ");
    std::string cmd_string = input_string.substr(0, pos);
    input_string.erase(0, pos);
    
    command cmd;
    cmd.type = commandMenu[cmd_string];
    switch (cmd.type){
        case commandType::LOGIN:
            cmd.value = inputLogin(input_string);
            break;
        case commandType::GET:
            std::vector<std::string> tmp = readDir(input_string);
            cmd.value = &tmp;
            break;
    }

    return cmd;
}

#endif