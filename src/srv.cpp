#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <sys/socket.h>

#include "server.h"
#include "chatlib.h"

client::client(sptr<IO::socket> sock, CryptoPP::RSA::PublicKey cli_public_key)
	:sock(sock)
{
	name = "generic_name";
	end_requested = false;
	this->cli_public_key = cli_public_key;
	e = CryptoPP::RSAES_OAEP_SHA_Encryptor(cli_public_key);
}

client::~client(){
	disconnect();
}

void client::disconnect(){
	end_requested = true;
	try{
		s_op.close(sock);
	}catch(const std::runtime_error &e){
		std::cerr << "error closing socket for " <<name << ":" << e.what();
	}
	if(thread && thread->joinable()){
		thread->join();
	}
}

bool client::get_end_requested() const{
	return end_requested;
}

void client::set_end_requested(bool requested){
	this->end_requested = requested;
}

void client::set_name(const std::string &name){
	this->name = name;
}

std::string client::get_name() const{
	return name;
}

void client::set_role(const role rl){
	this->rl = rl;
}

role client::get_role() const{
	return rl;
}

std::string client::recv() const{
	return chatlib::recv(sock, s_op);
}

void client::send(const std::string &mes){
	std::string encr;
	CryptoPP::StringSource ss1(mes, true,
		new CryptoPP::PK_EncryptorFilter(rng, e,
			new CryptoPP::StringSink(encr)));
	chatlib::send(encr, sock, s_op);
}

void client::set_thread(sptr<std::thread> thread){
	this->thread = thread;
}

sptr<std::thread> client::get_thread(){
	return thread;
}


//===========================================================//
server::server(const unsigned int max)
	:max(max),end_requested(false),ended(false)
{
	CryptoPP::InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(rng, 1024);

	private_key = CryptoPP::RSA::PrivateKey(params);
	srv_public_key = CryptoPP::RSA::PublicKey(params);
	d = CryptoPP::RSAES_OAEP_SHA_Decryptor(private_key);
}

server::~server(){
	end();
	if(accept_thread.joinable()){
		accept_thread.join();
	}
}

std::string server::_decode(const std::string &mes){
	std::string message;
	CryptoPP::StringSource ss2(mes, true,
		new CryptoPP::PK_DecryptorFilter(rng, d,
			new CryptoPP::StringSink(message)));
	return message;
}

sptr<client> server::accept_user(){
	sptr<IO::socket> temp;
	sptr<client> cli;
	try{
		temp = s_op.accept(this->sock);
		{//send key
		std::string srv_pub_key_encr;
		CryptoPP::StringSink ss(srv_pub_key_encr);
		srv_public_key.DEREncode(ss);
		chatlib::send(srv_pub_key_encr, temp, s_op);
		}
		CryptoPP::RSA::PublicKey cli_public_key;
		{//recv key;
		const auto cli_pub_key_encr = chatlib::recv(temp, s_op);
		CryptoPP::StringSource ss(cli_pub_key_encr, true);
		cli_public_key.BERDecode(ss);
		}
		cli = std::make_shared<client>(temp, cli_public_key);
		const auto name = _decode(cli->recv());
		cli->set_name(name);
		if(clients.size() >= max){
			print_mes(std::string("**can't accept ")+name+", server is full**");
			cli->disconnect();
			return nullptr;
		}
	}catch(std::runtime_error &e){
		if(end_requested){
			return nullptr;
		}
		throw std::runtime_error(std::string("can't accept client, ") + e.what());
	}catch(...){
		if(end_requested){
			return nullptr;
		}
		throw std::runtime_error(std::string("some error occurred"));
	}
	return cli;
}

void server::add_user(sptr<client> cli){
	mt.lock();
	clients.emplace_back(cli);
	auto thr = std::make_shared<std::thread>(&server::_process, this, cli);
	cli->set_thread(thr);
	mt.unlock();
}

void server::print_mes(const std::string &name, const std::string &mes) const{
	std::cout<<construct(name, mes)<<'\n';
}

void server::print_mes(const std::string &mes) const{
	std::cout<<mes<<'\n';
}

std::string server::construct(const std::string &name, const std::string &mes) const{
	return std::string("[")+name+"]: "+mes;
}

void server::send_to(sptr<client> cli, const std::string &mes){
	mt.lock();
	try{
		cli->send(mes);
	}catch(std::runtime_error &e){
		std::cerr<<"[ERROR]:can't send message to client, "<<
			e.what();
	}
	mt.unlock();
}

void server::remove_user(sptr<client> cli){
	mt.lock();
	auto it = std::find(clients.begin(), clients.end(), cli);
	if(it == clients.end()){
		std::cerr<<"[ERROR]:requested to remove unknown user\n";
		return;
	}
	clients.erase(it);
	mt.unlock();
}

sptr<client> server::get_user(const std::string &name){
	sptr<client> user = nullptr;
	mt.lock();
	const auto cli = std::find_if(clients.begin(), clients.end(), 
		[&](const auto &cli){
			return (cli->get_name() == name);	
		});
	if(cli != clients.end()){
		user = *cli;
	}
	mt.unlock();
	return user;
}

void server::broadcast(const std::string &mes){
	mt.lock();
	for(auto &cl:clients){
		try{
			cl->send(mes);
		}catch(std::runtime_error &e){
			std::cerr<<"[ERROR]:can't send message to client, "<<
				e.what();
		}catch(...){
			std::cerr<<"[ERROR]:some error occurred\n";
		}
	}
	mt.unlock();
}

void server::_process(sptr<client> cli){
	while(!cli->get_end_requested()){
		const std::string name = cli->get_name();
		std::string mes;
		try{
			const auto enc = cli->recv();
			mes = _decode(enc);
		}catch(std::runtime_error &e){
			if(end_requested){
				break;
			}
			std::cerr<<"[ERROR]:can't recieve message, "<<e.what();
		}
		print_mes(name, mes);

		mt.lock();
		auto it = mp.find(mes);
		mt.unlock();
		if(it != mp.end()){
			const auto trg = it->second;
			cqueue->push(trg);
			cqueue->push(cli->get_name());
		}

		const std::string str = construct(name, mes);
		try{
			broadcast(str);
		}catch(std::runtime_error &e){
			if(end_requested){
				break;
			}
			std::cerr<<"[ERROR]:can't recieve message, "<<e.what();
		}
	}
}

void server::end(){
	end_requested = true;
	if(ended){
		return;
	}
	mt.lock();
	try{
		s_op.shutdown(sock);
		s_op.close(sock);
	}catch(const std::runtime_error &e){
		print_mes(std::string("**ERROR: Can't end server: ")+e.what());
	}
	clients.clear();
	mt.unlock();
	ended = true;
}

void server::init(const int port){
	sock = s_op.create(AF_INET, SOCK_STREAM, 0);
	s_op.bind(sock, port);
	s_op.listen(sock, max);
	accept_thread = std::thread(&server::accept_user, this);
}

std::vector<sptr<client>> server::get_users(){
	std::vector<sptr<client>> users;
	mt.lock();
	users = this->clients;
	mt.unlock();
	return users;
}

int server::get_users_count(){
	mt.lock();
	auto count = clients.size();
	mt.unlock();
	return count;
}

int server::get_users_max(){
	return this->max;
}

bool server::get_end_requested(){
	return end_requested;
}

void server::add_callback(std::string command, std::string func_name){
	mp[command] = func_name;
}

void server::pass_cqueue(sptr<concurrent_queue<std::string>> cqueue){
	this->cqueue = cqueue;
}
