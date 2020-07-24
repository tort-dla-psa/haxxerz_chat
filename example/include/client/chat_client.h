#include "core/client/client.h"
#include "encrypt_engine/enc_eng.h"
#include "readerwriterqueue.h"

namespace net_send{
class chat_client:public client{
public:
    using ui_queue = moodycamel::BlockingReaderWriterQueue<std::string>;

    chat_client(ui_queue &mes_q,
        io_service &io_service, resolve_it endpoint);

    void on_recv(message &mes)override;
    void on_send(message &mes)override;
    void on_connect_err()override;
    void on_connected()override;
    void on_disconnected()override;
private:
    ui_queue &m_mes_q;
    enc_eng m_enc_eng;
};

};