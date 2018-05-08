#include "net.hpp"

namespace net {

Messenger::Messenger(TcpStream& ts) {
    streamfd = ts;
    buff = "";
}

Messenger::~Messenger() {
    
}
}