#include <iostream>
#include <chrono>
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
        std::thread out_thr([&c](){
            while(c.connected()){
                while(c.msgs_count() != 0){
                    message mes;
                    if(!c.msg(mes)){
                        break;
                    }
                    std::cout<<mes.get_str()<<std::endl;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); //FIXME
            }
        });

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
        if(out_thr.joinable()){
            out_thr.join();
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
