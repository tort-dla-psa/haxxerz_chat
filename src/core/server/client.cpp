#include "core/server/client.h"

using namespace net_send;

session::session(sock &&socket, server &srv_ref)
    :client(),
    m_socket(std::move(socket)),
    m_srv_ref(srv_ref)
{}

void session::start() {
    p_do_read_header();
}

void session::send(const message& msg){
    bool write_in_progress = !m_write_msgs.empty();
    m_write_msgs.emplace_back(msg);
    if (!write_in_progress) {
        p_do_write();
    }
}

void session::p_do_read_header() {
    auto self = shared_from_this();
    auto lbd = [this, self](auto ec, size_t) {
        if (!ec) {
            p_do_read_body();
        } else {
            m_srv_ref.on_error(self);
        }
    };
    boost::asio::async_read(m_socket,
        boost::asio::buffer(m_read_msg.header(), message::header_len),
        lbd
    );
}

void session::p_do_read_body() {
    auto self = shared_from_this();
    auto lbd = [this, self](auto ec, size_t) {
        if (!ec) {
            on_recv(m_read_msg);
            p_do_read_header();
        } else {
            m_srv_ref.on_error(self);
        }
    };
    auto buf = boost::asio::buffer(m_read_msg.data(), m_read_msg.datalen());
    boost::asio::async_read(m_socket, std::move(buf), lbd);
}

void session::p_do_write() {
    auto self = shared_from_this();
    auto lbd = [this, self](auto ec, size_t) {
        if (!ec) {
            m_write_msgs.pop_front();
            if (!m_write_msgs.empty()) {
                p_do_write();
            }
        } else {
            m_srv_ref.on_error(self);
        }
    };
    auto buf = boost::asio::buffer(m_write_msgs.front().header(),
            m_write_msgs.front().bytelen());
    boost::asio::async_write(m_socket, std::move(buf), lbd);
}