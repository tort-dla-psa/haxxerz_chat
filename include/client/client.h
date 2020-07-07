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
    bool connected()const;
    void disconnect();
	void send(const message &mes);

    bool msg(message &mes);
    size_t msgs_count();
private:
    void do_read_header();
    void do_read_body();
    void do_write();

    bool m_connected;
    chat_mes_queue m_write_msgs;
    std::mutex m_read_mt, m_write_mt;
    chat_mes_queue m_read_msgs;
    tcp_ns::socket m_sock;
    boost::asio::io_service& m_io_service;
};
