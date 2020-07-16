#include <iostream>
#include "chat_server.h"

using namespace net_send;

chat_server::chat_server(io_service& io_context, const endpoint& endp)
    :server(io_context, endp)
{}

void chat_server::on_connect(client_ptr sess){
    std::cout<<"got connection\n";
}
void chat_server::on_error(client_ptr sess){
    std::cerr<<"someone disconnected\n";
}
void chat_server::on_recv(client_ptr sess, const message &msg){
    for (auto &participant:m_participants){
        if(sess == participant){
            continue; //skip sender
        }
        participant->send(msg);
    }
}
void chat_server::on_accept(socket sock, client_ptr &sess){
    auto ip_str = sock.remote_endpoint().address().to_string();
    std::cout<<"got accept:"<<ip_str<<"\n";
    auto chat_sess = std::make_shared<chat_session>(std::move(sock), *this);
    sess = chat_sess;
    chat_sess->start();
}