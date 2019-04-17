#include <iostream>
#include <thread>
#include <chrono>

#include "server.h"

void _remove(sptr<server> srv){
	while(srv->get_users_count() != 0){
		auto users = srv->get_users();
		if(srv->get_end_requested()){
			break;
		}
		for(auto &u:users){
			if(u->get_end_requested()){
				srv->print_mes("**"+u->get_name()+" disconnected**");
				srv->remove_user(u);
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

int main(int argc, char* argv[]){
	sptr<server> srv(new server(10));
	srv->init(atoi(argv[1]));
	auto admin = srv->accept_user();
	srv->send_to(admin, "welcome! you are an admin from now on");
	admin->set_role(role::admin);
	srv->add_user(admin);
	std::thread thr(_remove, srv);
	while(srv->get_users_count()){
		if(srv->get_end_requested()){
			break;
		}
		auto cli = srv->accept_user();
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
	return 0;
}
