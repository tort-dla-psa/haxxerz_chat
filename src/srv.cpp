#include <iostream>
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
	connected = true;
}

client::~client(){
	disconnect();
}

void client::disconnect(){
	if(connected){
		s_op.shutdown(sock);
		s_op.close(sock);
		connected = false;
	}
}

bool client::get_status() const{
	return connected;
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
	:max(max),ended(false),end_requested(false)
{}

server::~server(){
	end();
	if(accept_thread.joinable()){
		accept_thread.join();
	}
}

sptr<client> server::try_accept(){
	if(clients.size() >= max){
		std::cerr<<"server is full\n";
		return nullptr;
	}
	sptr<IO::socket> temp;
	try{
		temp = s_op.accept(this->sock);
	}catch(std::runtime_error &e){
		std::cerr<<e.what();
		throw std::runtime_error("can't accept client");
	}
	return std::make_shared<client>(temp);
}

void server::accept_admin(){
	sptr<client> cli = try_accept();
	if(!cli){
		return;
	}
	cli->set_name(cli->recv());
	cli->set_role(role::admin);
	mt.lock();
	auto thr = std::make_shared<std::thread>(&server::process, this, cli);
	cli->set_thread(thr);
	clients.emplace_back(cli);
	mt.unlock();
}

void server::accept_user(){
	while(!ended){
		sptr<client> cli = try_accept();
		if(!cli){ return; }
		std::string name = cli->recv();
		cli->set_name(name);
		cli->set_role(role::user);
		for(const sptr<client> &cl:clients){
			if(cl->get_name() == name){
				cli->send("SRV: user exists");
				cli->disconnect();
				break;
			}
		}
		mt.lock();
		auto thr = std::make_shared<std::thread>(&server::process, this, cli);
		cli->set_thread(thr);
		clients.emplace_back(cli);
		mt.unlock();
	}
}

void server::print_mes(const std::string &name, const std::string &mes) const{
	std::cout<<construct(name, mes)<<'\n';
}

std::string server::construct(const std::string &name, const std::string &mes) const{
	return std::string("[")+name+"]: "+mes;
}

void server::process(sptr<client> cli){
	const std::string name = cli->get_name();
	while(cli->get_status()){
		const std::string mes = cli->recv();
		print_mes(name, mes);
		const std::string str = construct(name, mes);
		if(mes == CHAT_EXIT_CMD){
			mt.lock();
			cli->disconnect();
			mt.unlock();
		}else if(mes == CHAT_SHUTDOWN_CMD && cli->get_role() == role::admin){
			end_requested = true;
		}
		for(const auto &cl:clients){
			if(cl->get_status()){
				cl->send(str);
			}
		}
	}
}

void server::end(){
	if(ended){
		return;
	}
	ended = true;
	mt.lock();
	for(auto &cl:clients){
		cl->disconnect();
	}
	clients.clear();
	s_op.shutdown(sock);
	s_op.close(sock);
	mt.unlock();
}

void server::init(const int port){
	sock = s_op.create(AF_INET, SOCK_STREAM, 0);
	s_op.bind(sock, port);
	s_op.listen(sock, max);
}

void server::work(){
	accept_admin();
	accept_thread = std::thread(&server::accept_user, this);
	while(!end_requested){
		std::this_thread::sleep_for(std::chrono::seconds(1));
		mt.lock();
		int count = 0;
		auto it = clients.begin();
		while(it != clients.end()){
			if(!(*it)->get_status()){
				auto th = (*it)->get_thread();
				if(th->joinable()){
					th->join();
				}
				it = clients.erase(it);
			}else{
				it++;
			}
		}
		mt.unlock();
	}
	end();
}
