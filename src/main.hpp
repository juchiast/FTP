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
    {"rmdir", commandType::RMKDIR},
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

struct fileCommand{
    std::string remote = "";
    std::string localFile = "";
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
    if (str == "") return dirFiles;
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

fileCommand* inputListFile(std::string str){
    dirList* dirFiles = readDir(str);
    fileCommand* ls = new fileCommand;
    ls->remote = dirFiles->arrDir[0];
    ls->localFile = dirFiles->arrDir[1];
    delete dirFiles;
    return ls;
}

std::string* inputDir(std::string str){
    dirList* dirFiles = readDir(str);
    std::string* path = new std::string;
    *path =  dirFiles->arrDir[0];
    delete dirFiles;
    return path;
}

fileCommand* inputPut(std::string str){
    dirList* dirFiles = readDir(str);
    fileCommand* iput = new fileCommand;

    std::cout << dirFiles->numDir << std::endl;
    if (dirFiles->numDir == 0){
        iput->localFile = myReadline("(local-file) ");
        iput->remote = myReadline("(remote-file) ");
        return iput;    
    }

    iput->remote = dirFiles->arrDir[1];
    if (dirFiles->numDir > 1){
        iput->localFile = dirFiles->arrDir[0];
    } else iput->localFile = iput->remote;

    delete dirFiles;
    return iput;
}

fileCommand* inputGet(std::string str){
    dirList* dirFiles = readDir(str);
    fileCommand* iget = new fileCommand;
    iget->remote = dirFiles->arrDir[0];
    iget->localFile = dirFiles->arrDir[1];
    delete dirFiles;
    return iget;
}

command readCommand(){
    std::string input_string;
    input_string = myReadline("ftp> ");
//Lay yeu cau
    int pos = input_string.find(" ");
    if (pos <= 0) pos = input_string.length();
    std::string cmd_string = input_string.substr(0, pos);
    input_string.erase(0, pos + 1);
    
    command cmd;
    //std::cout << cmd_string << std::endl;
    if (commandMenu.find(cmd_string) == commandMenu.end()){
        cmd.type = commandType::ERROR;
        return cmd;
    } else cmd.type = commandMenu[cmd_string];

    switch (cmd.type){
        //login [ip] [username] [pass]
        case commandType::LOGIN:
            cmd.value = inputLogin(input_string);
            return cmd;
        
        //ls [remote-directory] [local-file]
        case commandType::LIST_FILE:
            cmd.value = inputListFile(input_string);
            return cmd;
        
        //put [local-file] [remote]
        case commandType::PUT:
            cmd.value = inputPut(input_string);
            return cmd;
        //get [remote] [local]
        case commandType::GET:
            cmd.value = inputGet(input_string);
            return cmd;

        case commandType::CD:
        case commandType::LCD:
        case commandType::DELETE:
        case commandType::MKDIR:
        case commandType::RMKDIR:
            cmd.value = inputDir(input_string);
            return cmd;

        //mput mget
        case commandType::MPUT:
        case commandType::MGET:
        case commandType::MDELETE:
            cmd.value = readDir(input_string);    
            return cmd;
        
        default:
            return cmd;
    }
}

#endif