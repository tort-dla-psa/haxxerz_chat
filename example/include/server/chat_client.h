#pragma once
#include "core/server/client.h"
#include "server/server.h"
//#include "encrypt_engine/enc_eng.h"

namespace net_send{

class chat_session:public session{
public:
    using sock = boost::asio::ip::tcp::socket;

    chat_session(sock &&socket, server &srv_ref);
    void on_recv(message &msg)override;
    //class enc_eng enc_eng;
};

};