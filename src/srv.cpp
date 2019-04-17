#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <sys/socket.h>

#include "server.h"
#include "chatlib.h"

client::client(sptr<IO::socket> sock)
	:sock(sock)
{
	name = "generic_name";
	end_requested = false;
}

client::~client(){
	disconnect();
}

void client::disconnect(){
	end_requested = true;
	s_op.close(sock);
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
	chatlib::send(mes, sock, s_op);
}

void client::set_thread(sptr<std::thread> thread){
	this->thread = thread;
}

sptr<std::thread> client::get_thread(){
	return thread;
}


//===========================================================//
server::server(const unsigned int max)
	:max(max),end_requested(false)
{}

server::~server(){
	end();
	if(accept_thread.joinable()){
		accept_thread.join();
	}
}

sptr<client> server::accept_user(){
	if(clients.size() >= max){
		throw std::runtime_error("server is full");
	}
	sptr<IO::socket> temp;
	sptr<client> cli;
	try{
		temp = s_op.accept(this->sock);
		cli = std::make_shared<client>(temp);
		cli->send("enter username\n");
		const auto name = cli->recv();
		cli->set_name(name);
		mt.lock();
		if(std::find_if(clients.begin(), clients.end(), 
			[&](const auto &cli){
				return (cli->get_name() == name);	
			}) != clients.end())
		{
			cli->send("SRV: user exists");
			cli->disconnect();
		}
		mt.unlock();
	}catch(std::runtime_error &e){
		throw std::runtime_error(std::string("can't accept client, ") + e.what());
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

void server::remove_user(sptr<client> cli){
	auto it = std::find(clients.begin(), clients.end(), cli);
	if(it == clients.end()){
		std::cerr<<"[ERROR]:requested to remove unknown user\n";
		return;
	}
	clients.erase(it);
}

void server::broadcast(const std::string &mes){
	for(auto &cl:clients){
		try{
			cl->send(mes);
		}catch(std::runtime_error &e){
			std::cerr<<"[ERROR]:can't send message to client, "<<
				e.what();
		}
	}
}

void server::_process(sptr<client> cli){
	while(true){
		const std::string name = cli->get_name();
		std::string mes;
		try{
			mes = cli->recv();
		}catch(std::runtime_error &e){
			std::cerr<<"[ERROR]:can't recieve message, "<<e.what();
			break;
		}
		print_mes(name, mes);
		if(mes == CHAT_EXIT_CMD){
			const std::string str = "**"+name+" quit**";
			mt.lock();
			remove_user(cli);
			broadcast(str);
			mt.unlock();
			break;
		}else if(mes == CHAT_SHUTDOWN_CMD && cli->get_role() == role::admin){
			end_requested = true;
			break;
		}
		mt.lock();
		const std::string str = construct(name, mes);
		broadcast(str);
		mt.unlock();
	}
}

void server::end(){
	mt.lock();
	clients.clear();
	s_op.shutdown(sock);
	s_op.close(sock);
	mt.unlock();
}

void server::init(const int port){
	sock = s_op.create(AF_INET, SOCK_STREAM, 0);
	s_op.bind(sock, port);
	s_op.listen(sock, max);
	accept_thread = std::thread(&server::accept_user, this);
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
