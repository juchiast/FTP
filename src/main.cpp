#include "ftp.hpp"
#include <iostream>

int main() {
    ftp::Ftp f;
    f.login("10.1.1.100", 21, "user", "pass");
    f.login("10.1.1.100", 21, "usexr", "pass");
    f.login("10.1.1.100", 21, "user", "pass");
    return 0;
}
