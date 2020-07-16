#include "client.h"
#include "chatlib.h"
#include <boost/asio/buffer.hpp>
#include <boost/asio/read.hpp>
#include <vector>
#include <iostream>

using namespace net_send;

chat_session::chat_session(sock socket, server &srv_ref)
    :m_socket(std::move(socket)),
    m_srv_ref(srv_ref)
{}

void chat_session::start() {
    auto self = shared_from_this();
    m_srv_ref.join(std::move(self));
    do_read_header();
}

void chat_session::deliver(const message& msg){
    bool write_in_progress = !m_write_msgs.empty();
    m_write_msgs.push_back(msg);
    if (!write_in_progress) {
        do_write();
    }
}

void chat_session::do_read_header() {
    auto self = shared_from_this();
    auto lbd = [this, self](auto ec, size_t) {
        if (!ec) {
            std::cout<<"read msg header, datalen:"<<m_read_msg.datalen()<<std::endl;
            do_read_body();
        } else {
            m_srv_ref.process_error(self);
        }
    };
    boost::asio::async_read(m_socket,
        boost::asio::buffer(m_read_msg.header(), message::header_len),
        lbd
    );
}

void chat_session::do_read_body() {
    auto self = shared_from_this();
    auto lbd = [this, self](auto ec, size_t) {
        if (!ec) {
            std::cout<<"read msg body, mes:"<<m_read_msg.get_str()<<std::endl;
            m_srv_ref.deliver(m_read_msg);
            do_read_header();
        } else {
            m_srv_ref.process_error(self);
        }
    };
    boost::asio::async_read(m_socket,
        boost::asio::buffer(m_read_msg.data(), m_read_msg.datalen()),
        lbd
    );
}

void chat_session::do_write() {
    auto self = shared_from_this();
    auto lbd = [this, self](auto ec, size_t) {
        if (!ec) {
            auto &mes = m_write_msgs.front();
            std::cout<<"wrote msg:"<<mes.get_str()
                <<" bytelen:"<<mes.bytelen()
                <<std::endl;
            m_write_msgs.pop_front();
            if (!m_write_msgs.empty()) {
                do_write();
            }
        } else {
            m_srv_ref.process_error(self);
        }
    };
    boost::asio::async_write(m_socket,
        boost::asio::buffer(m_write_msgs.front().header(),
            m_write_msgs.front().bytelen()),
        lbd
    );
}
