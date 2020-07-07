#include <mutex>
#include <sys/socket.h>
#include <thread>
#include <ncurses.h>
#include <iostream>
#include "chatlib.h"
#include "client.h"

using chat_mes_queue = client::chat_mes_queue;
using tcp_ns = client::tcp_ns;

client::client(boost::asio::io_service& io_service,
    tcp_ns::resolver::iterator endpoint_it)
    :m_sock(io_service),
    m_io_service(io_service)
{
    m_read_msgs.resize(1); //reserve place for 1 message
    boost::asio::async_connect(m_sock, endpoint_it,
        [this](auto ec, tcp_ns::resolver::iterator){
            m_connected = true;
            if (!ec) {
                do_read_header();
            }
    });
}

bool client::connected()const{
    return m_connected;
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
void client::disconnect(){
    m_io_service.post([this]() {
        m_sock.close();
        m_connected = false;
    });
    //m_sock.close(); //ISSUE:why not just this?
}

void client::send(const message &mes){
    m_io_service.post(
        [this, mes]() {
            bool write_in_progress = !m_write_msgs.empty();
            m_write_msgs.emplace_back(mes);
            if (!write_in_progress) {
                do_write();
            }
    });
}

void client::do_read_header(){
    auto lbd = [this](auto ec, size_t) {
        if (!ec) {
            //std::cout<<"read msg header, datalen:"
            //  <<m_read_msg.datalen()<<std::endl;
            do_read_body();
        } else {
            m_sock.close();
        }
    };
    auto buf = boost::asio::buffer(m_read_msgs.back().header(),
            message::header_len);
    boost::asio::async_read(m_sock, std::move(buf), lbd);
}

void client::do_read_body(){
    auto lbd = [this](auto ec, size_t ) {
          if (!ec) {
            //std::cout<<"read msg body, mes:"
            //  <<m_read_msg.get_str()<<std::endl;
            std::unique_lock lock(m_read_mt);
            m_read_msgs.emplace_back(); //adds next message
            do_read_header();
          } else {
            m_sock.close();
          }
    };
    auto buf = boost::asio::buffer(m_read_msgs.back().data(),
            m_read_msgs.back().datalen());
    boost::asio::async_read(m_sock, std::move(buf), lbd);
}

void client::do_write(){
    auto lbd = [this](auto ec, size_t) {
        if (!ec) {
            /*
            auto &mes = m_write_msgs.front();
            std::cout<<"wrote msg:"<<mes.get_str()
                <<" bytelen:"<<mes.bytelen()
                <<std::endl;
            */
            m_write_msgs.pop_front();
            if (!m_write_msgs.empty()) {
                do_write();
            }
        } else {
            m_sock.close();
        }
    };
    auto buf = boost::asio::buffer(m_write_msgs.front().header(),
            m_write_msgs.front().bytelen());
    boost::asio::async_write(m_sock, std::move(buf), lbd);
}
