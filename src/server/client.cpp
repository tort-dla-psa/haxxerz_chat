#include "client.h"
#include "chatlib.h"
#include <vector>

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
    using read = boost::asio::async_read;
    using buf = boost::asio::buffer;
    auto self = shared_from_this();
    read(m_socket, buf(&m_read_msg.header, message::header_len),
        [this, self](boost::system::error_code ec, size_t) {
            if (!ec) {
                protocol::decode_header(m_read_msg);
                do_read_body();
            } else {
                m_srv_ref.process_error(self);
            }
    });
}

void chat_session::do_read_body() {
    using read = boost::asio::async_read;
    using buf = boost::asio::buffer;
    auto self = shared_from_this();

    read(m_socket, buf(m_read_msg.data.data(), m_read_msg.data.size()),
        [this, self](boost::system::error_code ec, size_t) {
            if (!ec) {
                m_srv_ref.deliver(m_read_msg);
                do_read_header();
            } else {
                m_srv_ref.process_error(self);
            }
    });
}

void chat_session::do_write() {
    using write = boost::asio::async_write;
    using buf = boost::asio::buffer;
    auto self = shared_from_this();
    auto &mes_to_write = m_write_msgs.front();
    auto bytes = protocol::encode(mes_to_write);

    write(m_socket, buf(bytes.data(), bytes.size()),
        [this, self](boost::system::error_code ec, size_t) {
            if (!ec) {
                m_write_msgs.pop_front();
                if (!m_write_msgs.empty()) {
                    do_write();
                }
            } else {
                m_srv_ref.process_error(self);
            }
    });
}
