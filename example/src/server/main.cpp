#include <iostream>
#include <memory>
#include "chat_server.h"

int main(int argc, char* argv[]){
    int port = 1337;
	try {
		if(argc > 1){
			port = std::atoi(argv[1]);
		}

		boost::asio::io_service io_service;
		using namespace boost::asio::ip;
		tcp::endpoint endpoint(tcp::v4(), port);
		net_send::chat_server srv(io_service, endpoint);
		io_service.run();
	} catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
	}
	return 0;
}
