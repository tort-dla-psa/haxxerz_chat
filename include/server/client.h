#pragma once
#include <queue>
#include <future>
#include <cryptopp/osrng.h>
#include <cryptopp/sha.h>
#include <cryptopp/rsa.h>
#include <cryptopp/pssr.h>
#include <cryptopp/cryptlib.h>
#include "socket.hpp"
#include "encrypt_engine.h"
#include "semaphore.hpp"
#include "datatypes.hpp"

class client{
    size_t id;
	std::string m_name;
    std::shared_ptr<io::socket> m_sock;
	role m_rl;
    encrypt_engine enc_eng;
    std::queue<message> m_msg_to_send;

	CryptoPP::RSA::PublicKey m_pub_key;
	CryptoPP::RSAES_OAEP_SHA_Encryptor m_enc;
	CryptoPP::AutoSeededRandomPool m_rng;
public:
	client(std::shared_ptr<io::socket> sock,
        CryptoPP::RSA::PublicKey cli_public_key);
	~client();

	void disconnect();
	void set_name(const std::string &name);
	const std::string& name() const;
	void set_role(const role rl);
	role get_role() const;
    std::future<struct message> recv(semaphore &s) const;
	void send(const struct message &mes);
    std::future<void> send_msgs(semaphore &s);
};

