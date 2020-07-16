#include <algorithm>
#include "core/server/server.h"
#include "core/server/client.h"

using namespace net_send;

server::server(io_service& io_context, const endpoint& endpoint)
    :m_acceptor(io_context, endpoint),
    m_sock(io_context)
{
    p_do_accept();
}

void server::process_connect(client_ptr sess){
    m_participants.insert(sess);
    on_connect(sess);
}
void server::process_disconnect(client_ptr sess){
    m_participants.erase(sess);
    on_disconnect(sess);
}
void server::p_do_accept() {
    auto lbd = [this](auto ec) {
        if (!ec) {
            client_ptr sess;
            on_accept(std::move(this->m_sock), sess);
            process_connect(sess);
        }
        p_do_accept();
    };
    m_acceptor.async_accept(m_sock, lbd);
}