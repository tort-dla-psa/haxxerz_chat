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
	bool end_requested;
	role rl;
	sptr<std::thread> thread;
public:
	client(sptr<IO::socket> sock);
	~client();
	void disconnect();
	void set_end_requested(bool requested);
	bool get_end_requested() const;
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
	std::atomic_bool end_requested;
	vect<sptr<client>> clients;
	std::thread accept_thread;
	std::mutex mt;
	sptr<client> _try_accept();
	void _process(sptr<client> cli);
public:
	server(const unsigned int max);
	~server();
	void init(const int port);
	void end();
	sptr<client> accept_user();
	void add_user(sptr<client> cli);
	void set_muted(sptr<client> cli, bool muted);
	void set_banned(sptr<client> cli, bool banned);
	void remove_user(sptr<client> cli);
	void print_mes(const std::string &mes) const;
	void print_mes(const std::string &name, const std::string &mes) const;
	void print_err(const std::string &err) const;
	void broadcast(const std::string &mes);
	std::string construct(const std::string &name, const std::string &mes) const;
	int get_users_count();
	int get_users_max();
	bool get_end_requested();
};

#endif
