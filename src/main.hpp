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

//Menu cua cac lenh
std::map<const std::string, const enum commandType> commandMenu = {
    {"login", commandType::LOGIN },
    {"help", commandType::HELP},
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
    {"active", commandType::ACTIVE},
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

struct listFile{
    std::string path = "";
    std::string outputFile = "";
};

struct dirList{
    std::string arrDir[maxFile];
    int numDir = 0;
};

//Kieu du dieu command
struct command{
    enum commandType type;
    void* value = NULL;
    ~command(){
        if (!value) delete value;
    }
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
    if (tmp) info->password = tmp;

    return info;
}

dirList* readDir(const std::string str){
    dirList* dirFiles = new dirList;
    int splitPos;
    int currPos = 0;
    int i = 0;
    int prevPos = 0;

    do{ 
        if (str[currPos] == '\"'){
            splitPos =  str.find('\"', currPos + 1);
            prevPos++;
        }
        else 
            splitPos = str.find(" ",currPos);   

        currPos = splitPos;
        dirFiles->arrDir[i++] = str.substr(prevPos, currPos - prevPos);
        do{
            currPos++;
        } while (str[currPos] == ' ' && currPos < str.length());
        prevPos = currPos;
    } while( splitPos >= 0);
    
    dirFiles->numDir = i;
    return dirFiles;
}

listFile* inputListFile(std::string str){
    dirList* dirFiles = readDir(str);
    listFile* ls = new listFile;
    ls->path = dirFiles->arrDir[0];
    ls->outputFile = dirFiles->arrDir[1];
    delete dirFiles;
    return ls;
}

command readCommand(){
    std::string input_string;
    input_string = myReadline("ftp>");
//Lay yeu cau
    int pos = input_string.find(" ");
    std::string cmd_string = input_string.substr(0, pos);
    input_string.erase(0, pos + 1);
    
    command cmd;
    if (commandMenu.find(cmd_string) == commandMenu.end()){
        cmd.type = commandType::ERROR;
        return cmd;
    } else cmd.type = commandMenu[cmd_string];

    switch (cmd.type){
        case commandType::LOGIN:
            cmd.value = inputLogin(input_string);
            return cmd;
        
        case commandType::LIST_FILE:
            cmd.value = inputListFile(input_string);
            return cmd;
    }

    return command();
}

#endif