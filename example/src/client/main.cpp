#include <iostream>
#include <chrono>
#include <memory>
#include <thread>
#include <atomic>
#include <iostream>

#include "chat_client.h"
#include "ui.h"

int main(int argc, char* argv[]){
    using namespace boost::asio::ip;
    std::atomic_bool end=false;

    std::unique_ptr<class ui> ui = std::make_unique<ui_imgui>();
    ui::queue_t& recv_q = ui->queue_recv();
    ui::queue_t& send_q = ui->queue_send();
    try {
        if (argc != 3) {
            std::cerr << "Usage: chat_client <host> <port>\n";
            return 1;
        }

        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        auto endpoint_iterator = resolver.resolve({ argv[1], argv[2] });
        net_send::chat_client c(recv_q, io_service, endpoint_iterator);
        std::thread t([&io_service](){ io_service.run(); });
        std::thread cli_thr([&c, &send_q, &recv_q, &end](){
            while(!end){
                std::string str;
                while(send_q.try_dequeue(str)){
                    net_send::message mes;
                    mes.set_str(str);
                    c.send(mes);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); //FIXME
            }
            c.disconnect();
        });
        ui->start(); //loops
        end = true;

        if(t.joinable()){
            t.join();
        }
        if(cli_thr.joinable()){
            cli_thr.join();
        }
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
