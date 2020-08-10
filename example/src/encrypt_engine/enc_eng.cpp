//#include "encrypt_engine/enc_eng.h"

using namespace net_send;

enc_eng::enc_eng(){
    CryptoPP::InvertibleRSAFunction params;
    params.GenerateRandomWithKeySize(m_rnd, 4096);
    this->m_prv_key = std::make_unique<prv_key>();
    this->m_pub_key = std::make_shared<pub_key>();
    this->m_dec = std::make_unique<decryptor>(*this->m_prv_key);
}
void enc_eng::set_bobs_pub_key(std::shared_ptr<pub_key> bobs_key){
    this->m_bobs_pub_key = bobs_key;
    this->m_enc = std::make_unique<encryptor>(*bobs_key);
}
void enc_eng::set_bobs_pub_key(const std::string &bobs_key){
    this->m_bobs_pub_key = std::make_shared<enc_eng::pub_key>();
    m_bobs_pub_key->BERDecode(CryptoPP::StringSource(bobs_key, true).Ref());
    this->m_enc = std::make_unique<encryptor>(*m_bobs_pub_key);
}
std::shared_ptr<const enc_eng::pub_key> enc_eng::get_bobs_pub_key()const{ return m_bobs_pub_key; }
std::shared_ptr<const enc_eng::pub_key> enc_eng::get_pub_key()const{ return m_pub_key; }
std::string enc_eng::encrypt(const std::string &mes){
    if(!this->m_bobs_pub_key){
        auto mes = "bob's public key not set in enc_eng";
        throw std::runtime_error(mes);
    }
    std::string result;
    CryptoPP::StringSource(mes, true,
        new CryptoPP::PK_EncryptorFilter(m_rnd, *m_enc,
            new CryptoPP::StringSink(result)
        )
    );
    return result;
}
std::string enc_eng::decrypt(const std::string &mes){
    std::string result;
    CryptoPP::StringSource(mes, true,
        new CryptoPP::PK_DecryptorFilter(m_rnd, *m_dec,
            new CryptoPP::StringSink(result)
        )
    );
    return result;
}