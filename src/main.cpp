#include "ftp.hpp"
#include <iostream>

int main() {
    ftp::Ftp f;
    f.login("10.1.1.100", 21, "user", "pass");
    std::cout << std::endl;
    f.list();
    return 0;
}
