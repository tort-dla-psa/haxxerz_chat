#include "encrypt_engine/enc_eng.h"

using namespace net_send;
using key = enc_eng::key;
using pub_key = enc_eng::pub_key;
using prv_key = enc_eng::prv_key;

enc_eng::enc_eng(){
    CryptoPP::InvertibleRSAFunction params;
    params.GenerateRandomWithKeySize(m_rnd.rnd, 4096);
    this->m_prv_key = std::make_unique<prv_key>();
    this->m_prv_key->val = CryptoPP::RSA::PrivateKey(params);
    this->m_pub_key = std::make_shared<pub_key>();
    this->m_pub_key->val = CryptoPP::RSA::PublicKey(params);
    this->m_dec.dec = CryptoPP::RSAES_OAEP_SHA_Decryptor(this->m_prv_key->val);
}
void enc_eng::set_bobs_pub_key(std::shared_ptr<pub_key> bobs_key){
    this->m_bobs_pub_key = bobs_key;
    this->m_enc.enc = CryptoPP::RSAES_OAEP_SHA_Encryptor(bobs_key->val);
}
std::shared_ptr<const pub_key> enc_eng::get_bobs_pub_key()const{ return m_bobs_pub_key; }
std::shared_ptr<const pub_key> enc_eng::get_pub_key()const{ return m_pub_key; }
std::string enc_eng::encrypt(const std::string &mes){
    if(!this->m_bobs_pub_key){
        auto mes = "bob's public key not set in enc_eng";
        throw std::runtime_error(mes);
    }
    std::string result;
    CryptoPP::StringSource(mes, true,
        new CryptoPP::PK_EncryptorFilter(m_rnd.rnd, m_enc.enc,
            new CryptoPP::StringSink(result)
        )
    );
    return result;
}
std::string enc_eng::decrypt(const std::string &mes){
    std::string result;
    CryptoPP::StringSource(mes, true,
        new CryptoPP::PK_DecryptorFilter(m_rnd.rnd, m_dec.dec,
            new CryptoPP::StringSink(result)
        )
    );
    return result;
}