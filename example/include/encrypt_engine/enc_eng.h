#pragma once
#include <memory>
#include "crypto++/osrng.h"
#include "crypto++/rsa.h"
#include "crypto++/sha.h"
#include "crypto++/filters.h"
#include "crypto++/secblock.h"

namespace net_send{

class enc_eng{
public:
    using pub_key = CryptoPP::RSA::PublicKey;
    using prv_key = CryptoPP::RSA::PrivateKey;
    using encryptor = CryptoPP::RSAES_OAEP_SHA_Encryptor;
    using decryptor = CryptoPP::RSAES_OAEP_SHA_Decryptor;
    using rand_gen = CryptoPP::AutoSeededRandomPool;
protected:
    std::unique_ptr<prv_key> m_prv_key;
    std::shared_ptr<pub_key> m_pub_key;
    std::shared_ptr<pub_key> m_bobs_pub_key;
    std::unique_ptr<decryptor> m_dec;
    std::unique_ptr<encryptor> m_enc;
    rand_gen m_rnd;
public:
    enc_eng();
    void set_bobs_pub_key(std::shared_ptr<pub_key> bobs_key);
    void set_bobs_pub_key(const std::string &bobs_key);
    std::shared_ptr<const pub_key> get_bobs_pub_key()const;
    std::shared_ptr<const pub_key> get_pub_key()const;
    std::string encrypt(const std::string &mes);
    std::string decrypt(const std::string &mes);
};

};