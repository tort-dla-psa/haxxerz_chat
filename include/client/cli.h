#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <cryptopp/osrng.h>
#include <cryptopp/sha.h>
#include <cryptopp/rsa.h>
#include <cryptopp/cryptlib.h>
#include "socket.hpp"

class client{
	std::shared_ptr<io::socket> sock;
	bool ended;
	std::string name;
	void _generate_keys();
	std::string _encrypt(const std::string &mes);
	std::string _decrypt(const std::string &mes);
	CryptoPP::RSA::PrivateKey private_key;
	CryptoPP::RSA::PublicKey my_public_key, srv_public_key;
	CryptoPP::RSAES_OAEP_SHA_Encryptor e;
	CryptoPP::RSAES_OAEP_SHA_Decryptor d;
	CryptoPP::AutoSeededRandomPool rng;
public:
	client();
	~client();
	void connect(const std::string &ip, const int port);
	void disconnect();
	void send(const std::string &mes);
	void set_name(const std::string &name);
	std::string get_name()const;
	std::string get_mes();
	bool get_ended();
};
