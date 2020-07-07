#include <iostream>
#include <chrono>
#include <memory>
#include <thread>
#include <atomic>
#include <iostream>

#include "chatlib.h"
#include "client.h"
#include "ui.h"

int main(int argc, char* argv[]){
    using namespace boost::asio::ip;

    std::unique_ptr<class ui> ui;
    std::thread ui_render_thread([&ui](){
        ui = std::make_unique<ui_imgui>();
        ui->start();
    });
    try {
        if (argc != 3) {
            std::cerr << "Usage: chat_client <host> <port>\n";
            return 1;
        }

        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        auto endpoint_iterator = resolver.resolve({ argv[1], argv[2] });
        client c(io_service, endpoint_iterator);

        std::thread out_thr([&c, &ui](){
            while(c.connected()){
                while(c.msgs_count() != 0){
                    message mes;
                    if(!c.msg(mes)){
                        break;
                    }
                    ui->add_recieved_msg(mes);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); //FIXME
            }
        });

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
        if(out_thr.joinable()){
            out_thr.join();
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    if(ui_render_thread.joinable()){
        ui_render_thread.join();
    }
    return 0;
}
