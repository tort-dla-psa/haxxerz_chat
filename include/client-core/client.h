#pragma once
#include <boost/asio/detail/type_traits.hpp>
#include <boost/asio/io_service.hpp>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <cryptopp/osrng.h>
#include <cryptopp/sha.h>
#include <cryptopp/rsa.h>
#include <cryptopp/cryptlib.h>
#include <boost/asio.hpp>
#include "datatypes.hpp"

namespace net_send{

class client{
public:
    using chat_mes_queue =  std::deque<message>;
    using io_service = boost::asio::io_service;
    using resolve_it = boost::asio::ip::tcp::resolver::iterator;
    using socket = boost::asio::ip::tcp::socket;

	client(io_service& io_service, resolve_it endpoint);
    virtual ~client();

    virtual bool connected()const;
    virtual void disconnect();
	virtual void send(const message &mes);
    /* events */
    virtual void on_connected(){}
    virtual void on_disconnected(){}
    virtual void on_send_err(){}
    virtual void on_recv_err(){}
    virtual void on_connect_err(){}
    virtual void on_send(message &mes){}
    virtual void on_recv(message &mes){}
    /* messages access */
    virtual bool msg(message &mes);
    virtual size_t msgs_count();
private:
    void p_do_read_header();
    void p_do_read_body();
    void p_do_write();

    bool m_connected;
    chat_mes_queue m_write_msgs;
    std::mutex m_read_mt, m_write_mt;
    chat_mes_queue m_read_msgs;
    socket m_sock;
    io_service& m_io_service;
};

};