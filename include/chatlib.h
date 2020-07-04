#include <sys/time.h>
#include <sys/types.h>
#include <string>
#include <cstring>
#include <vector>
#include <memory>
#include "socket.hpp"
#include "datatypes.hpp"

namespace protocol{
void encode_header(message &msg){
}

std::vector<uint8_t> encode(const message &msg){
    std::vector<uint8_t> result;
    result.resize(msg.header_len + msg.data.size());
    auto ptr = result.data();
    memcpy(ptr, (uint8_t*)&msg.header, msg.header_len);
    ptr += sizeof(msg.header_len);
    memcpy(ptr, (uint8_t*)msg.data.data(), msg.data.size());
    return result;
}

void decode_header(message &m){
    auto ptr = &m.header;
    uint32_t data_len;
    memcpy((uint8_t*)&data_len, ptr, sizeof(data_len));
    ptr += sizeof(data_len);
    m.data.resize(data_len, '\0');
}

/*
message decode(const std::vector<uint8_t> &data){
    message result;
    auto ptr = data.data();
    memcpy((uint8_t*)&result.id, ptr, sizeof(result.id));
    ptr += sizeof(result.id);
    uint32_t data_len;
    memcpy((uint8_t*)&data_len, ptr, sizeof(data_len));
    ptr += sizeof(data_len);
    result.data.resize(data_len);
    memcpy((uint8_t*)&result.data[0], ptr, data_len);
    return result;
}*/

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
