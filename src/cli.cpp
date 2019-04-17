#include <sys/socket.h>
#include <thread>
#include <ncurses.h>
#include "chatlib.h"
#include "cli.h"

template <typename T> using vect = std::vector<T>;

client::client(){}

client::~client(){
	if(connected){
		disconnect();
	}
}

void client::connect(const std::string &ipstr, const int port){
	sock = s_op.create(PF_INET, SOCK_STREAM, 0);
	s_op.connect(sock, ipstr, port);
	connected = true;
	read_thr = std::thread(&client::_read, this);
}

void client::disconnect(){
	s_op.shutdown(sock);
	s_op.close(sock);
	connected = false;
}

void client::_read(){
	while(connected){
		const auto mes = chatlib::recv(sock, s_op);
		if(mes.empty()){
			continue;
		}
		mt.lock();
		messages.emplace_back(mes);
		mt.unlock();
	}
}

void client::set_name(const std::string &name){
	this->name = name;
}

std::string client::get_name()const{
	return name;
}

void client::send(const std::string &mes)const{
	chatlib::send(mes, sock, s_op);
}

std::vector<std::string> client::get_mes_queue(){
	mt.lock();
	const auto temp = std::move(this->messages);
	mt.unlock();
	return temp;
}
