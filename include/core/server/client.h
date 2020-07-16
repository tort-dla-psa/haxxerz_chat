#pragma once
#include <memory>
#include <queue>
#include <vector>
#include <boost/asio/write.hpp>
#include <boost/asio.hpp>
#include "datatypes.hpp"
#include "core/server/server.h"

namespace net_send{

class client {
public:
    virtual ~client() {}
    virtual void send(const message& msg) = 0;
};

using client_ptr = std::shared_ptr<client>;

class session:public client,
    public std::enable_shared_from_this<session>
{
public:
    using sock = boost::asio::ip::tcp::socket;
    session(sock &&socket, server &srv_ref);

    void start();
    void send(const message &msg)override;
    virtual void on_recv(message &msg){};
private:
    void p_do_read_header();
    void p_do_read_body();
    void p_do_write();

    sock m_socket;
    message m_read_msg;
    std::deque<message> m_write_msgs;
protected:
    server& m_srv_ref;
};

};