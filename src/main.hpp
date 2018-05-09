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

//Cac cau truc yeu cau
enum command_type{
    LOGIN,
    HELP
};

//Menu cua cac lenh
std::map<std::string, enum command_type> command_menu = {
    { "login", LOGIN },
    { "help", HELP}
};

//Cau truc login
struct login{
    std::string ip;
    uint16_t port = 21;
    std::string user_name;
    std::string password;
};

//Kieu du dieu command
struct command{
    enum command_type type;
    void* value = NULL;
};

login* input_login(std::string str){
    login* info = new login();

    if (str != ""){
        info->ip = str;    
    } else {
        info->ip = readline("(to) ");
    }

    //connect 
    /*auto stream_client = net::TcpStream::connect(info->ip, info->port);
    if (stream_client) {
        std:: cout << "Connect to " << info->ip << " success!" << std::endl;
    }*/

    info->user_name = readline("Name: ");
    info->password = getpass("Pass: ");

    //std::cout << "Name: " << info->user_name << std:: endl;
    //std::cout << "Pass: " << info->password << std::endl;
    return info;
}

command read_command(){
    std::string input_string;
    input_string = readline("ftp>");
//Lay yeu cau
    int pos = input_string.find(" ");
    std::string cmd_string = input_string.substr(0, pos);
    input_string.erase(0, pos);
    
    command cmd;
    cmd.type = command_menu[cmd_string];
    switch (cmd.type){
        case LOGIN:
            cmd.value = input_login(input_string);
            break;
    }

    return cmd;
}

#endif