#include <future>
#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <queue>

#include "server.h"
#include "chatlib.h"

//===========================================================//
server::server(const size_t &max)
	:m_semaphore(m_threads_num), m_threads_num(4),
    m_max(max), m_end_requested(false)
{
	CryptoPP::InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(m_rng, 1024);

	m_prv_key = CryptoPP::RSA::PrivateKey(params);
	m_pub_key = CryptoPP::RSA::PublicKey(params);
	m_dec = CryptoPP::RSAES_OAEP_SHA_Decryptor(m_prv_key);
}

server::~server(){
	end();
	if(m_accept_thread.joinable()){
		m_accept_thread.join();
	}
}

std::string server::p_decode(const std::string &mes){
	std::string message;
	CryptoPP::StringSource ss2(mes, true,
		new CryptoPP::PK_DecryptorFilter(m_rng, m_dec,
			new CryptoPP::StringSink(message)));
	return message;
}

void server::p_accept(){
    while(!m_end_requested){
        std::shared_ptr<io::socket> temp;
        std::shared_ptr<client> cli;
        try{
            //accept user
            temp = std::make_shared<io::socket>(this->m_sock->accept());
            {//init key pair and send public key to user
                std::string srv_pub_key_encr;
                CryptoPP::StringSink ss(srv_pub_key_encr);
                m_pub_key.DEREncode(ss);
                protocol::send(srv_pub_key_encr, temp);
            }
            CryptoPP::RSA::PublicKey cli_public_key;
            {//recv user public key;
                const auto cli_pub_key_encr = protocol::recv(temp);
                CryptoPP::StringSource ss(cli_pub_key_encr, true);
                cli_public_key.BERDecode(ss);
            }
            cli = std::make_shared<client>(temp, cli_public_key);
            if(m_clients.size() >= m_max){
                auto mes = std::string("can't accept,server is full");
                std::cerr<<mes<<std::endl;
                cli->disconnect();
            }
        }catch(std::runtime_error &e){
            auto mes = std::string("can't accept client, ") + e.what();
            std::cerr<<mes<<std::endl;
        }catch(...){
            auto mes = std::string("some error occurred");
            std::cerr<<mes<<std::endl;
        }
    }
}


void server::print_mes(const std::string &name, const std::string &mes) const{
	auto tmp = std::string("[")+name+"]: "+mes;
	std::cout<<tmp<<'\n';
}

void server::print_mes(const std::string &mes) const{
	std::cout<<mes<<'\n';
}

void server::send_to(std::shared_ptr<client> cli, const std::string &mes){
    message m;
    m.message = mes;
	m_mt.lock();
	try{
		cli->send(m);
	}catch(std::runtime_error &e){
		std::cerr<<"[ERROR]:can't send message to client, "<<
			e.what();
	}
	m_mt.unlock();
}

void server::remove_user(std::shared_ptr<client> cli){
	m_mt.lock();
	auto it = std::find(m_clients.begin(), m_clients.end(), cli);
	if(it == m_clients.end()){
		std::cerr<<"[ERROR]:requested to remove unknown user\n";
		return;
	}
	m_clients.erase(it);
	m_mt.unlock();
}

std::shared_ptr<client> server::get_user(const std::string &name){
	std::shared_ptr<client> user = nullptr;
	m_mt.lock();
	const auto cli = std::find_if(m_clients.begin(), m_clients.end(), 
		[&](const auto &cli){
			return (cli->name() == name);	
		});
	if(cli != m_clients.end()){
		user = *cli;
	}
	m_mt.unlock();
	return user;
}

void server::broadcast(const std::string &mes){
    message m;
    m.message = mes;
	m_mt.lock();
	for(auto &cl:m_clients){
		try{
			cl->send(m);
		}catch(std::runtime_error &e){
			std::cerr<<"[ERROR]:can't send message to client, "<<
				e.what();
		}catch(...){
			std::cerr<<"[ERROR]:some error occurred\n";
		}
	}
	m_mt.unlock();
}

void server::end(){
	m_end_requested = true;
	m_mt.lock();
	try{
        m_sock->close();
	}catch(const std::runtime_error &e){
		print_mes(std::string("**ERROR: Can't end server: ")+e.what());
	}
	m_clients.clear();
	m_mt.unlock();
}

void server::init(const int port){
    m_sock = std::make_shared<io::socket>(AF_INET, SOCK_STREAM, 0);
    m_sock->listen(m_max);
	m_accept_thread = std::thread(&server::p_accept, this);
}

void server::start(){
    while(!m_end_requested){
        std::queue<std::future<struct message>> ftrs;
        for(auto &usr:m_clients){
            ftrs.emplace(usr->recv(m_semaphore));
        }
        while(!ftrs.empty()){
            auto ftr = std::move(ftrs.front());
            auto mes = std::move(ftr.get());
            for(auto &mention:mes.mentions){
                for(auto &usr:m_clients){
                    if(usr->name() == mention){
                        usr->send(mes);
                        break;
                    }
                }
            }
        }
        for(auto &usr:m_clients){
            usr->send_msgs(m_semaphore);
        }
    }
}

std::vector<std::shared_ptr<client>> server::users(){
	std::vector<std::shared_ptr<client>> users;
	m_mt.lock();
	users = this->m_clients;
	m_mt.unlock();
	return users;
}

int server::users_count(){
	m_mt.lock();
	auto count = m_clients.size();
	m_mt.unlock();
	return count;
}

int server::users_max(){
	return this->m_max;
}

bool server::end_requested(){
	return m_end_requested;
}

void server::add_callback(const std::string &command, const std::string &func_name){
	mp[command] = func_name;
}
