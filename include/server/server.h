#pragma once

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

#include "client.h"
#include "socket.hpp"
#include "encrypt_engine.h"
#include "semaphore.hpp"
#include "datatypes.hpp"

class server{
	const size_t m_max;
    size_t m_threads_num;
    bool m_end_requested;

    std::shared_ptr<io::socket> m_sock;
    std::vector<std::shared_ptr<client>> m_clients;
	std::thread m_accept_thread;
	std::mutex m_mt;

	CryptoPP::RSA::PrivateKey m_prv_key;
	CryptoPP::RSA::PublicKey m_pub_key;
	CryptoPP::RSAES_OAEP_SHA_Decryptor m_dec;
	CryptoPP::AutoSeededRandomPool m_rng;

	std::map<std::string, std::string> mp;
    semaphore m_semaphore;

	std::string p_decode(const std::string &mes);
    void p_accept();
public:
	server(const size_t &max);
	~server();
	void init(const int port);
    void start();
	void end();

	void send_to(std::shared_ptr<client> cli, const std::string &mes);
	void remove_user(std::shared_ptr<client> cli);
	std::shared_ptr<client> get_user(const std::string &name);
	void print_mes(const std::string &mes) const;
	void print_mes(const std::string &name, const std::string &mes) const;
	void print_err(const std::string &err) const;
	void broadcast(const std::string &mes);
	std::vector<std::shared_ptr<client>> users();
	int users_count();
	int users_max();
	bool end_requested();
	void add_callback(const std::string &command,
           const std::string &func_name);
};
