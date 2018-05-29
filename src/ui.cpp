#include "ui.hpp"
#include <iostream>
#include <map>
#include <readline/history.h>
#include <readline/readline.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
using std::cout;
using std::endl;

using namespace ui;

// Menu cua cac lenh
const static std::map<std::string, enum commandType> commandMenu = {
    {"login", commandType::LOGIN},   {"help", commandType::HELP},
    {"ls", commandType::LIST_FILE},  {"dir", commandType::LIST_FILE},
    {"put", commandType::PUT},       {"mput", commandType::MPUT},
    {"get", commandType::GET},       {"mget", commandType::MGET},
    {"cd", commandType::CD},         {"lcd", commandType::LCD},
    {"delete", commandType::DELETE}, {"mdelete", commandType::MDELETE},
    {"mkdir", commandType::MKDIR},   {"rmdir", commandType::RMKDIR},
    {"pwd", commandType::PWD},       {"passive", commandType::PASSIVE},
    {"active", commandType::ACTIVE}, {"quit", commandType::EXIT},
    {"exit", commandType::EXIT}};

static std::string myReadline(const char *prompt) {
    char *tmp = readline(prompt);
    std::string str;
    if (tmp)
        str.assign(tmp);
    free(tmp);
    return str;
}

static login *inputLogin(const std::string str) {
    login *info = new login();
    char *tmp;

    if (str != "") {
        info->ip = str;
    } else {
        info->ip = myReadline("(to) ");
    }

    info->userName = myReadline("Name: ");

    tmp = getpass("Pass: ");
    if (tmp)
        info->password = tmp;

    return info;
}

static dirList *readDir(std::string str) {
    dirList *dirFiles = new dirList;
    if (str == "")
        return dirFiles;
    while (str[0] == ' ') str.erase(0, 1);
    while (str[str.length() - 1] == ' ') str.erase(str.length() - 1, 1);
    int splitPos;
    int currPos = 0;
    int i = 0;
    int prevPos = 0;

    do {
        if (str[currPos] == '\"') {
            splitPos = str.find('\"', currPos + 1);
            prevPos++;
        } else
            splitPos = str.find(" ", currPos);

        currPos = splitPos;
        dirFiles->arrDir[i++] = str.substr(prevPos, currPos - prevPos);
        do {
            currPos++;
        } while (str[currPos] == ' ' && currPos < str.length());
        prevPos = currPos;
    } while (splitPos >= 0);

    dirFiles->numDir = i;
    //std::cout << i << endl;
    return dirFiles;
}

static fileCommand *inputListFile(std::string str) {
    dirList *dirFiles = readDir(str);
    fileCommand *ls = new fileCommand;
    ls->remote = dirFiles->arrDir[0];
    ls->localFile = dirFiles->arrDir[1];
    delete dirFiles;
    return ls;
}

static std::string *inputDir(std::string str) {
    dirList *dirFiles = readDir(str);
    std::string *path = new std::string;
    *path = dirFiles->arrDir[0];
    delete dirFiles;
    return path;
}

static fileCommand *inputPut(std::string str) {
    dirList *dirFiles = readDir(str);
    fileCommand *iput = new fileCommand;

    //std::cout << dirFiles->numDir << std::endl;
    if (dirFiles->numDir == 0) {
        iput->localFile = myReadline("(local-file) ");
        iput->remote = myReadline("(remote-file) ");
        return iput;
    }

    iput->localFile = dirFiles->arrDir[0];
    if (dirFiles->numDir > 1) {
        iput->remote = dirFiles->arrDir[1];
    } else
        iput->remote = iput->localFile;

    delete dirFiles;
    return iput;
}

static fileCommand *inputGet(std::string str) {
    dirList *dirFiles = readDir(str);
    fileCommand *iget = new fileCommand;

    //std::cout << dirFiles->numDir << std::endl;
    if (dirFiles->numDir == 0) {
        iget->localFile = myReadline("(remote-file) ");
        iget->remote = myReadline("(local-file) ");
        return iget;
    }

    iget->remote = dirFiles->arrDir[0];
    if (dirFiles->numDir > 1) {
        iget->localFile = dirFiles->arrDir[1];
    } else
        iget->localFile = iget->remote;

    delete dirFiles;
    return iget;
}

static command readCommand() {
    std::string input_string = myReadline("ftp> ");
    // Lay yeu cau
    int pos = input_string.find(" ");
    if (pos <= 0)
        pos = input_string.length();
    std::string cmd_string = input_string.substr(0, pos);
    input_string.erase(0, pos + 1);

    command cmd;
    if (commandMenu.find(cmd_string) == commandMenu.end()) {
        cmd.type = commandType::ERROR;
        return cmd;
    } else {
        cmd.type = commandMenu.at(cmd_string);
    }

    switch (cmd.type) {
    // login [ip] [username] [pass]
    case commandType::LOGIN:
        cmd.value = inputLogin(input_string);
        return cmd;

    // ls [remote-directory] [local-file]
    case commandType::LIST_FILE:
        cmd.value = inputListFile(input_string);
        return cmd;

    // put [local-file] [remote]
    case commandType::PUT:
        cmd.value = inputPut(input_string);
        return cmd;
    // get [remote] [local]
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

    // mput mget
    case commandType::MPUT:
    case commandType::MGET:
    case commandType::MDELETE:
        cmd.value = readDir(input_string);
        return cmd;

    default:
        return cmd;
    }
}

namespace ui {
int run(void *_ftp) {
    auto f = (ftp::Ftp *)_ftp;
    command cmd;
    cmd = readCommand();

    switch (cmd.type) {
    case commandType::LOGIN: {
        login *lg = (login *)cmd.value;
        f->login(lg->ip, lg->port, lg->userName, lg->password);
        delete lg;
        break;
    }
    case commandType::LIST_FILE: {
        fileCommand *ls = (fileCommand *)cmd.value;
        f->list(ls->remote);
        delete ls;
        break;
    }
    case commandType::PUT: {
        fileCommand *path = (fileCommand *)cmd.value;
        f->store(path->localFile, path->remote);
        delete path;
        break;
    }
    case commandType::GET: {
        fileCommand *path = (fileCommand *)cmd.value;
        f->retrieve(path->localFile, path->remote);
        delete path;
        break;
    }
    case commandType::MPUT: {
        dirList* dl = (dirList*)cmd.value;
        for (int i = 0; i < dl->numDir; i++){
            auto confirm = myReadline((dl->arrDir[i] + "? ").c_str());
            if (confirm == "y")
                f->store(dl->arrDir[i], dl->arrDir[i]);
        }
        break;
    }
    case commandType::MGET: {
        dirList* dl = (dirList*)cmd.value;
        for (int i = 0; i < dl->numDir; i++) {
            auto confirm = myReadline((dl->arrDir[i] + "? ").c_str());
            if (confirm == "y")
                f->retrieve(dl->arrDir[i], dl->arrDir[i]);
        }
        break;
    }
    case commandType::CD: {
        std::string *path = (std::string *)cmd.value;
        f->chdir(*path);
        delete path;
        break;
    }
    case commandType::LCD: {
        std::string *path = (std::string *)cmd.value;
        f->local_chdir(*path);
        delete path;
        break;
    }
    case commandType::DELETE: {
        std::string *path = (std::string *)cmd.value;
        f->remove(*path);
        delete path;
        break;
    }
    case commandType::MDELETE: {
        dirList *ld = (dirList *)cmd.value;
        for (int i = 0; i < ld->numDir; i++) {
            f->remove(ld->arrDir[i]);
        }
        delete ld;
        break;
    }
    case commandType::MKDIR: {
        std::string *path = (std::string *)cmd.value;
        f->mkdir(*path);
        delete path;
        break;
    }
    case commandType::RMKDIR: {
        std::string *path = (std::string *)cmd.value;
        f->rmdir(*path);
        delete path;
        break;
    }
    case commandType::PWD:
        f->pwd();
        break;
    case commandType::EXIT:
        f->quit();
        return 2;
    case commandType::PASSIVE:
        f->set_passive();
        break;
    case commandType::ACTIVE:
        f->set_active();
        break;

    default:
        cout << "?Invalid command" << endl;
    }

    return 0;
}

} // namespace ui
