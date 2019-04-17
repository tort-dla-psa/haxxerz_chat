#include <iostream>

#include "server.h"

int main(int argc, char* argv[]){
	server srv(10);
	srv.init(atoi(argv[1]));
	auto admin = srv.accept_user();
	admin->set_role(role::admin);
	srv.add_user(admin);
	while(srv.get_users_count() != 0 || !srv.get_end_requested()){
		auto cli = srv.accept_user();
		srv.print_mes("**"+cli->get_name()+" connected**");
		srv.add_user(std::move(cli));
	}
	srv.end();
	return 0;
}
