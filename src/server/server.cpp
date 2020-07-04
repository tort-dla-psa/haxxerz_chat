#include <future>
#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <queue>

#include "server.h"
#include "client.h"
#include "chatlib.h"


namespace asio_ns = boost::asio;
using tcp_ns = server::tcp_ns;

server::server(asio_ns::io_context& io_context,
    const tcp_ns::endpoint& endpoint)
    :m_acceptor(io_context, endpoint)
{
    do_accept();
}

void server::join(chat_participant_ptr sess){
    m_participants.insert(sess);
}

void server::process_error(chat_participant_ptr sess){
    m_participants.erase(sess);
}

void server::deliver(const message &msg){
    for (auto &participant:m_participants)
        participant->deliver(msg);
}

void server::do_accept() {
    auto lbd = [this](auto ec, tcp_ns::socket socket) {
        if (!ec) {
            auto sess = std::make_shared<chat_session>
                (std::move(socket), *this);
            sess->start();
        }

        do_accept();
    };
    m_acceptor.async_accept(lbd);
}
