#include <sys/socket.h>
#include <thread>
#include <ncurses.h>
#include "chatlib.h"
#include "cli.h"

template <typename T> using vect = std::vector<T>;

client::client(){
	ended = false;
	_generate_keys();
}

client::~client(){
	disconnect();
}

void client::_generate_keys(){
	CryptoPP::InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(rng, 1024);

	private_key = CryptoPP::RSA::PrivateKey(params);
	my_public_key = CryptoPP::RSA::PublicKey(params);
	d = CryptoPP::RSAES_OAEP_SHA_Decryptor(private_key);
}

std::string client::_encrypt(const std::string &mes){
	std::string encr;
	CryptoPP::StringSource ss1(mes, true,
		new CryptoPP::PK_EncryptorFilter(rng, e,
			new CryptoPP::StringSink(encr)));
	return encr;
}

std::string client::_decrypt(const std::string &mes){
	std::string message;
	CryptoPP::StringSource ss2(mes, true,
		new CryptoPP::PK_DecryptorFilter(rng, d,
			new CryptoPP::StringSink(message)));
	return message;
}

void client::connect(const std::string &ipstr, const int port){
	sock = std::make_shared<io::socket>(PF_INET, SOCK_STREAM, 0);
	//s_op.connect(sock, ipstr, port);
	ended = false;
	{//recv key;
        const auto srv_pub_key_encr = protocol::recv(sock);
        CryptoPP::StringSource ss(srv_pub_key_encr, true);
        srv_public_key.BERDecode(ss);
        e = CryptoPP::RSAES_OAEP_SHA_Encryptor(srv_public_key);
	}

	{//send key
        std::string my_pub_key_encr;
        CryptoPP::StringSink ss(my_pub_key_encr);
        my_public_key.DEREncode(ss);
        protocol::send(my_pub_key_encr, sock);
	}
}

void client::disconnect(){
	if(ended){
		return;
	}
    sock->close();
	ended = false;
}

void client::set_name(const std::string &name){
	this->name = name;
}

std::string client::get_name()const{
	return name;
}

void client::send(const std::string &mes){
	const auto encr = _encrypt(mes);
	protocol::send(encr, sock);
}

std::string client::get_mes(){
	const auto mes = protocol::recv(sock);
	const auto decr = _decrypt(mes);
	return decr;
}

bool client::get_ended(){
	return ended;
}
