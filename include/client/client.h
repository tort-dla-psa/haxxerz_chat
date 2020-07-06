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

class client{
public:
    using chat_mes_queue =  std::deque<message>;
    using tcp_ns = boost::asio::ip::tcp; 

	client(boost::asio::io_service& io_service,
        tcp_ns::resolver::iterator endpoint_it);
    void disconnect();
	void send(const message &mes);
private:
    void do_read_header();
    void do_read_body();
    void do_write();

    message m_read_msg;
    std::vector<uint8_t> m_read_msg_header;
    chat_mes_queue m_write_msgs;
    chat_mes_queue m_queue;
    tcp_ns::socket m_sock;
    boost::asio::io_service& m_io_service;
};
