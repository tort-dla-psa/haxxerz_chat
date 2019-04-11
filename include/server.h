#ifndef SERV_H
#define SERV_H

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include "file.h"
#include "socket_op.h"

#define CHAT_EXIT_CMD "/exit"
#define CHAT_SHUTDOWN_CMD "/end"

template <typename T> using vect = std::vector<T>;
template <typename T> using uptr = std::unique_ptr<T>;
template <typename T> using sptr = std::shared_ptr<T>;

enum role{admin, user, muted, banned};

using namespace IO;

class client{
	std::string name;
	sptr<IO::socket> sock;
	socket_op s_op;
	bool connected;
	role rl;
	sptr<std::thread> thread;
public:
	client(sptr<IO::socket> sock);
	~client();
	void disconnect();
	bool get_status() const;
	void set_name(const std::string &name);
	std::string get_name() const;
	void set_role(const role rl);
	role get_role() const;
	std::string recv() const;
	void send(const std::string &mes);
	void set_thread(sptr<std::thread> thread);
	sptr<std::thread> get_thread();
};

class server{
	const unsigned int max;
	sptr<IO::socket> sock;
	socket_op s_op;
	bool ended;
	std::atomic_bool end_requested;
	vect<sptr<client>> clients;
	std::thread accept_thread;
	sptr<client> try_accept();
	void accept_admin();
	void accept_user();
	void mute_user(bool muted);
	void ban_user(bool banned);
	void print_mes(const std::string &name, const std::string &mes) const;
	std::string construct(const std::string &name, const std::string &mes) const;
	void process(sptr<client> cli);
	std::mutex mt;
public:
	server(const unsigned int max);
	~server();
	void end();
	void init(const int port);
	void work();
};

#endif
