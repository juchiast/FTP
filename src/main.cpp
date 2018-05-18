#include "ftp.hpp"
#include <iostream>

int main() {
    ftp::Ftp f;
    f.local_pwd();
    f.local_chdir("/home/qwe");
    f.local_pwd();
    return 0;
}
