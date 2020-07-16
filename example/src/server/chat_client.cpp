#include "chat_client.h"
#include <boost/asio/buffer.hpp>
#include <boost/asio/read.hpp>
#include <vector>
#include <iostream>

using namespace net_send;

chat_session::chat_session(sock &&socket, server &srv_ref)
    :session(std::move(socket), srv_ref)
{}

void chat_session::on_recv(message &msg){
    auto self = shared_from_this();
    m_srv_ref.on_recv(self, msg);
}