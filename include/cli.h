#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include "file.h"
#include "socket_op.h"

template <typename T> using sptr = std::shared_ptr<T>;
template <typename T> using vec = std::vector<T>;

using namespace IO;

class client{
	socket_op s_op;
	sptr<IO::socket> sock;
	bool connected;
	std::string name;
	vec<std::string> messages;
	std::thread read_thr;
	std::mutex mt;
	void _read();
public:
	client();
	~client();
	void connect(const std::string &ip, const int port);
	void disconnect();
	void send(const std::string &mes)const;
	void set_name(const std::string &name);
	std::string get_name()const;
	vec<std::string> get_mes_queue();
};
