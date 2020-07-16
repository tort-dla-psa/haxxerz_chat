#pragma once
#include <deque>
#include <string>
#include <memory>
#include <mutex>
#include <boost/asio/io_service.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <set>
#include "datatypes.hpp"

namespace net_send{

class client;
using client_ptr = std::shared_ptr<client>;
class session;

class server{
public:
    using io_service = boost::asio::io_service;
    using endpoint = boost::asio::ip::tcp::endpoint;
    using socket = boost::asio::ip::tcp::socket;
    using acceptor = boost::asio::ip::tcp::acceptor;

    server(io_service& io_context, const endpoint& endp);

    void process_connect(client_ptr sess);
    void process_disconnect(client_ptr sess);
    virtual void on_connect(client_ptr sess){};
    virtual void on_disconnect(client_ptr sess){};
    virtual void on_accept(socket sock, client_ptr &sess){};
    virtual void on_error(client_ptr sess){};
    virtual void on_recv(client_ptr sess, const message &msg){};
private:
    void p_do_accept();

    acceptor m_acceptor;
    socket m_sock;
protected:
    std::set<client_ptr> m_participants;
};

}