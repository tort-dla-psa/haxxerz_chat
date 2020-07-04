#pragma once

#include <deque>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <map>
#include <set>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cryptopp/osrng.h>
#include <cryptopp/sha.h>
#include <cryptopp/rsa.h>
#include <cryptopp/pssr.h>
#include <cryptopp/cryptlib.h>

#include "socket.hpp"
#include "encrypt_engine.h"
#include "semaphore.hpp"
#include "datatypes.hpp"

class chat_participant;
using chat_participant_ptr = std::shared_ptr<chat_participant>;
class chat_session{
public:
    void start();
};

class server{
public:
    using tcp_ns = boost::asio::ip::tcp;
    using chat_message_queue = std::deque<message>;

    server(boost::asio::io_context& io_context,
        const tcp_ns::endpoint& endpoint);

    void join(chat_participant_ptr sess);
    void process_error(chat_participant_ptr sess);
    void deliver(const message &msg);
private:
    void do_accept();

    tcp_ns::acceptor m_acceptor;
    std::set<chat_participant_ptr> m_participants;
    chat_message_queue recent_msgs_;
};
