#ifndef SERV_H
#define SERV_H

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <map>
#include <atomic>
#include <cryptopp/osrng.h>
#include <cryptopp/sha.h>
#include <cryptopp/rsa.h>
#include <cryptopp/pssr.h>
#include <cryptopp/cryptlib.h>

#include "concurrent_queue.h"
#include "file.h"
#include "socket_op.h"

template <typename T> using vect = std::vector<T>;
template <typename T> using uptr = std::unique_ptr<T>;
template <typename T> using sptr = std::shared_ptr<T>;

enum role{admin, user, muted, banned};

using namespace IO;

class client{
	std::string name;
	sptr<IO::socket> sock;
	socket_op s_op;
	std::atomic_bool end_requested;
	role rl;
	sptr<std::thread> thread;
	CryptoPP::RSA::PublicKey cli_public_key;
	CryptoPP::RSAES_OAEP_SHA_Encryptor e;
	CryptoPP::AutoSeededRandomPool rng;
public:
	client(sptr<IO::socket> sock, CryptoPP::RSA::PublicKey cli_public_key);
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
	std::atomic_bool end_requested, ended;
	vect<sptr<client>> clients;
	std::thread accept_thread;
	std::mutex mt;

	CryptoPP::RSA::PrivateKey private_key;
	CryptoPP::RSA::PublicKey srv_public_key;
	CryptoPP::RSAES_OAEP_SHA_Decryptor d;
	CryptoPP::AutoSeededRandomPool rng;

	sptr<client> _try_accept();
	void _process(sptr<client> cli);
	std::string _decode(const std::string &mes);

	std::map<std::string, std::string> mp;
	sptr<concurrent_queue<std::string>> cqueue;
public:
	server(const unsigned int max);
	~server();
	void init(const int port);
	void end();
	sptr<client> accept_user();
	void add_user(sptr<client> cli);
	void set_muted(sptr<client> cli, bool muted);
	void set_banned(sptr<client> cli, bool banned);
	void send_to(sptr<client> cli, const std::string &mes);
	void remove_user(sptr<client> cli);
	sptr<client> get_user(const std::string &name);
	void print_mes(const std::string &mes) const;
	void print_mes(const std::string &name, const std::string &mes) const;
	void print_err(const std::string &err) const;
	void broadcast(const std::string &mes);
	std::string construct(const std::string &name, const std::string &mes) const;
	std::vector<sptr<client>> get_users();
	int get_users_count();
	int get_users_max();
	bool get_end_requested();
	void add_callback(std::string command, std::string func_name);
	void pass_cqueue(sptr<concurrent_queue<std::string>> cqueue);
};

#endif
