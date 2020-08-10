#include <mutex>
#include <sys/socket.h>
#include <thread>
#include <iostream>
#include "chat_client.h"

using namespace net_send;
using chat_mes_queue = client::chat_mes_queue;

chat_client::chat_client(ui_queue &mes_q, io_service& io_service, resolve_it endpoint)
    :client(io_service, endpoint),
    m_mes_q(mes_q)
{
    m_got_bobs_pubkey = false;
}

void chat_client::on_recv(message &mes){
    if(!m_got_bobs_pubkey){
        m_got_bobs_pubkey = true;
        //auto str = mes.get_str();
        //m_enc_eng.set_bobs_pub_key(str);
    }
    auto ui_mes = "RECV:"+mes.get_str();
    m_mes_q.emplace(std::move(ui_mes));
}
void chat_client::on_send(message &mes){
    auto ui_mes = "SENT:"+std::move(mes.get_str());
    m_mes_q.emplace(std::move(ui_mes));
}
void chat_client::on_connect_err(){
    m_mes_q.emplace("CHAT:can't connect to a server");
}
void chat_client::on_connected(){
    m_mes_q.emplace("CHAT:successfully connected to a server");
}
void chat_client::on_disconnected(){
    m_mes_q.emplace("CHAT:disconnected from a server");
}