#include <sys/time.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include <memory>
#include "socket.hpp"

namespace protocol{
inline void send(const std::string &mes, std::shared_ptr<io::socket> sock){
    uint64_t len = mes.size();
    sock->write_all(len);
    sock->write_all(mes.data(), len);
}

inline std::string recv(std::shared_ptr<io::socket> sock){
    uint64_t length;
    auto rc = sock->read_all(length);
    if(rc == EAGAIN || rc == EWOULDBLOCK){
        return "";
    }
    std::string mes(length, ' ');
    sock->read_all(&mes.at(0), length);
    return mes;
}

namespace cmds{
const std::string exit = "/exit";
const std::string end = "/end";
};

};
