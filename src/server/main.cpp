#include <iostream>
#include <memory>
#include <unistd.h>
#include <thread>
#include <chrono>

#include "chatlib.h"
#include "server.h"

std::unique_ptr<server> srv;
std::mutex cb_mutex;

void on_exit_cmd(std::shared_ptr<client> sender){
}

void on_end_cmd(std::shared_ptr<client> sender){
}

using callback_t = std::function<void(std::shared_ptr<client>)>;
std::map<std::string, callback_t> callbacks;

int lastname=0;

void add_callback(const std::string &command, callback_t cb){
    auto lastname_str = std::to_string(lastname);
	callbacks[lastname_str] = std::bind(cb, std::placeholders::_1);
	srv->add_callback(command, std::to_string(lastname));
	lastname++;
}

int main(int argc, char* argv[]){
	srv = std::make_unique<server>(10);
    int port = 1337;
    if(argc > 1){
        port = std::atoi(argv[1]);
    }
	srv->init(port);
    srv->start();
	srv->end();
	return 0;
}
