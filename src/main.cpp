#include "net.hpp"
#include <iostream>

int main() {
    net::Ftp f;
    f.login("10.1.1.100", 21, "user", "pass");
    return 0;
}
