#include <iostream>
#include <memory>
#include <unistd.h>
#include <thread>
#include <chrono>

#include "chatlib.h"
#include "server.h"

int main(int argc, char* argv[]){
    int port = 1337;
	try {
		if(argc > 1){
			port = std::atoi(argv[1]);
		}

		boost::asio::io_service io_service;

/*
		std::list<server> servers;
		for (int i = 1; i < argc; ++i) {
		  tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
		  servers.emplace_back(io_service, endpoint);
		}

			io_service.run();
*/
			using namespace boost::asio::ip;
			tcp::endpoint endpoint(tcp::v4(), port);
			net_send::server srv(io_service, endpoint);
            io_service.run();
	} catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
	}
	return 0;
}
