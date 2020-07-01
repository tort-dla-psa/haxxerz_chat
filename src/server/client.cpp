#include "client.h"
#include "chatlib.h"

client::client(std::shared_ptr<io::socket> sock, CryptoPP::RSA::PublicKey cli_public_key)
	:m_sock(sock),
    m_pub_key(cli_public_key)
{
	m_name = "generic_name";
	m_enc = CryptoPP::RSAES_OAEP_SHA_Encryptor(cli_public_key);
}

client::~client(){
	disconnect();
}

void client::disconnect(){
    m_sock->close();
}


void client::set_name(const std::string &name){
	this->m_name = name;
}

const std::string& client::name() const{
	return m_name;
}

void client::set_role(const role rl){
	this->m_rl = rl;
}

role client::get_role() const{
	return m_rl;
}

std::future<struct message> client::recv(semaphore &s) const{
    auto task = [this, &s](){
        semaphore_controller sc(s);
        auto mes = protocol::recv(m_sock);
        message m;
        m.message = std::move(mes);
        return m;
    };
	return std::async(std::launch::async, std::move(task));
}

void client::send(const struct message &mes){
	std::string encr;
	CryptoPP::StringSource ss1(mes.message, true,
		new CryptoPP::PK_EncryptorFilter(m_rng, m_enc,
			new CryptoPP::StringSink(encr)));
	protocol::send(encr, m_sock);
}

std::future<void> client::send_msgs(semaphore &s){
    auto task = [this, &s](){
        semaphore_controller sc(s);
        while(!m_msg_to_send.empty()){
            auto mes = std::move(m_msg_to_send.front());
            protocol::send(mes.message, m_sock);
            m_msg_to_send.pop();
        }
    };
	return std::async(std::launch::async, std::move(task));
}

