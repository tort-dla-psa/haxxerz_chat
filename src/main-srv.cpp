#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>

#include "chatlib.h"
#include "server.h"
#include "concurrent_queue.h"

sptr<server> srv;
sptr<concurrent_queue<std::string>> cqueue;
std::mutex cb_mutex;

void _remove(){
	while(!srv->get_end_requested()){
		auto users = srv->get_users();
		for(auto &u:users){
			if(u->get_end_requested()){
				srv->print_mes("**"+u->get_name()+" disconnected**");
				srv->remove_user(u);
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void on_exit_cmd(sptr<client> sender){
	srv->remove_user(sender);
	srv->print_mes(std::string("**")+sender->get_name()+" requested exit**");
	srv->broadcast("**"+sender->get_name()+" quit**");
}

void on_end_cmd(sptr<client> sender){
	if(sender->get_role() != role::admin){
		sender->send("**You have no rights!**");
		return;
	}
	srv->broadcast("**"+sender->get_name()+" requested server to stop**");
	srv->end();
}

std::map<std::string, std::function<void(sptr<client>)>> callbacks;
int lastname=0;
void add_callback(const std::string &command, std::function<void(sptr<client>)> cb){
	callbacks[std::to_string(lastname)] = std::bind(cb, std::placeholders::_1);
	srv->add_callback(command, std::to_string(lastname));
	lastname++;
}

void cb_listen(){
	while(!srv->get_end_requested()){
		std::string trg;
		if(!cqueue->wait_pop(trg) && cqueue->exit_requested()){
			break;
		}
		std::string name;
		if(!cqueue->wait_pop(name) && cqueue->exit_requested()){
			break;
		}
		cb_mutex.lock();
		auto it = callbacks.find(trg);
		cb_mutex.unlock();
		if(it != callbacks.end()){
			(it->second)(srv->get_user(name));
		}
	}
}

int main(int argc, char* argv[]){
	srv = std::make_shared<server>(10);
	srv->init(atoi(argv[1]));
	cqueue = std::make_shared<concurrent_queue<std::string>>();
	srv->pass_cqueue(cqueue);
	std::thread cb_thread(cb_listen);
	add_callback(chatlib::cmd_exit, on_exit_cmd);
	add_callback(chatlib::cmd_end, on_end_cmd);
	sptr<client> admin;
	try{
		admin = srv->accept_user();
		admin->set_role(role::admin);
		srv->send_to(admin, "welcome! you are an admin from now on");
		srv->add_user(admin);
	}catch(const std::runtime_error &e){
		std::cerr<<e.what();
	}
	std::thread thr(_remove);
	while(!srv->get_end_requested()){
		sptr<client> cli;
		try{
			cli = srv->accept_user();
		}catch(const std::runtime_error &e){
			std::cerr<<e.what();
		}
		if(!cli && srv->get_end_requested()){
			break;
		}
		cli->set_role(role::user);
		if(srv->get_user(cli->get_name())){
			srv->send_to(cli, "error: user exists");
			cli->disconnect();
		}else{
			srv->send_to(cli, "welcome!");
			srv->print_mes("**"+cli->get_name()+" connected**");
			srv->add_user(std::move(cli));
		}
	}
	srv->end();
	if(thr.joinable()){
		thr.join();
	}
	if(cb_thread.joinable()){
		cb_thread.join();
	}
	return 0;
}
