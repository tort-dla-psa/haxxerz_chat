#pragma once
#include <boost/asio/write.hpp>
#include <memory>
#include <queue>
#include <future>
#include <cryptopp/osrng.h>
#include <cryptopp/sha.h>
#include <cryptopp/rsa.h>
#include <cryptopp/pssr.h>
#include <cryptopp/cryptlib.h>
#include <boost/asio.hpp>
#include "encrypt_engine.h"
#include "semaphore.hpp"
#include "datatypes.hpp"
#include "server.h"

class chat_participant {
public:
    virtual ~chat_participant() {}
    virtual void deliver(const message& msg) = 0;
};

using chat_participant_ptr = std::shared_ptr<chat_participant>;

class chat_session:public chat_participant,
    public std::enable_shared_from_this<chat_session>
{
public:
    using sock = boost::asio::ip::tcp::socket;
    chat_session(sock socket, server &srv_ref);

    void start();
    void deliver(const message& msg);
private:
    void do_read_header();
    void do_read_body();
    void do_write();

    sock m_socket;
    server& m_srv_ref;
    std::vector<uint8_t> m_read_msg_header;
    message m_read_msg;
    std::deque<message> m_write_msgs;
};
