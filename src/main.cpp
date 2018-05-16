#include "ftp.hpp"
#include <iostream>

int main() {
    ftp::Ftp f;
    f.login("127.0.0.1", 21, "user", "pass");
    std::cout << std::endl;
    f.list();
    return 0;
}
