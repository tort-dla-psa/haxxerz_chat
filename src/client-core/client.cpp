#include <mutex>
#include <sys/socket.h>
#include <thread>
#include <iostream>
#include "chatlib.h"
#include "client-core/client.h"

using namespace net_send;
using chat_mes_queue = client::chat_mes_queue;

client::client(io_service& io_service, resolve_it endpoint)
    :m_sock(io_service),
    m_io_service(io_service)
{
    m_read_msgs.resize(1); //reserve place for 1 message
    auto lbd_connect = [this](auto ec, resolve_it){
        m_connected = true;
        on_connected();
        if (!ec) {
            p_do_read_header();
        } else {
            on_connect_err();
        }
    };
    boost::asio::async_connect(m_sock, endpoint, lbd_connect);
}
client::~client(){}

bool client::connected()const{
    return m_connected;
}
void client::disconnect(){
    m_io_service.post([this]() {
        m_sock.close();
        m_connected = false;
        on_disconnected();
    });
    //m_sock.close(); //ISSUE:why not just this?
}
void client::send(const message &mes){
    auto lbd_send = [this, mes]() {
        std::unique_lock lock(m_write_mt);
        bool write_in_progress = !m_write_msgs.empty();
        m_write_msgs.emplace_back(mes);
        if (!write_in_progress) {
            p_do_write();
        }
    };
    m_io_service.post(lbd_send);
}
bool client::msg(message &mes){
    std::unique_lock lock(m_read_mt);
    if(m_read_msgs.size() < 2){
        //because empty or just reserved message
        return false;
    }
    mes = m_read_msgs.front();
    m_read_msgs.pop_front();
    return true;
}
size_t client::msgs_count(){
    std::unique_lock lock(m_read_mt);
    //note: reserved message does not count
    return m_read_msgs.size()-1;
}

/* privates */
void client::p_do_read_header(){
    auto lbd = [this](auto ec, size_t) {
        if (!ec) {
            p_do_read_body();
        } else {
            on_recv_err();
            m_sock.close();
        }
    };
    auto buf = boost::asio::buffer(m_read_msgs.back().header(),
            message::header_len);
    boost::asio::async_read(m_sock, std::move(buf), lbd);
}

void client::p_do_read_body(){
    auto lbd = [this](auto ec, size_t ) {
        if (!ec) {
            std::unique_lock lock(m_read_mt);
            on_recv(m_read_msgs.back());
            m_read_msgs.emplace_back(); //adds next message
            p_do_read_header();
        } else {
            on_recv_err();
            m_sock.close();
        }
    };
    auto buf = boost::asio::buffer(m_read_msgs.back().data(),
            m_read_msgs.back().datalen());
    boost::asio::async_read(m_sock, std::move(buf), lbd);
}

void client::p_do_write(){
    auto lbd = [this](auto ec, size_t) {
        if (!ec) {
            std::unique_lock lock(m_write_mt);
            on_send(m_write_msgs.front());
            m_write_msgs.pop_front();
            if (!m_write_msgs.empty()) {
                p_do_write();
            }
        } else {
            on_send_err();
            m_sock.close();
        }
    };
    auto buf = boost::asio::buffer(m_write_msgs.front().header(),
            m_write_msgs.front().bytelen());
    boost::asio::async_write(m_sock, std::move(buf), lbd);
}
