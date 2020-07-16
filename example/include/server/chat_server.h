#pragma once
#include "core/server/server.h"
#include "chat_client.h"

namespace net_send{

class chat_server:public server{
public:
    using chat_message_queue = std::deque<message>;

    chat_server(io_service& io_context, const endpoint& endp);

    void on_connect(client_ptr sess)override;
    void on_accept(socket sock, client_ptr &sess)override;
    void on_error(client_ptr sess)override;
    void on_recv(client_ptr sess, const message &msg)override;
private:
    chat_message_queue m_recent_msgs_;
};

};