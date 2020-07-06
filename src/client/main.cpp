#include <iostream>
#include <thread>
#include <atomic>
#include <iostream>

#include "chatlib.h"
#include "client.h"

int main(int argc, char* argv[]){
    using namespace boost::asio::ip;

    try {
        if (argc != 3) {
            std::cerr << "Usage: chat_client <host> <port>\n";
            return 1;
        }

        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        auto endpoint_iterator = resolver.resolve({ argv[1], argv[2] });
        client c(io_service, endpoint_iterator);

        std::thread t([&io_service](){ io_service.run(); });

        while (true){
            message mes;
            std::string str;
            std::getline(std::cin, str);
            mes.set_str(str);
            c.send(mes);
            if(mes.get_str() == "\\quit"){
                break;
            }
        }

        c.disconnect();
        if(t.joinable()){
            t.join();
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
