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
    struct key{};
    struct pub_key:key{ CryptoPP::RSA::PublicKey val; };
    struct prv_key:key{ CryptoPP::RSA::PrivateKey val; };
    struct encryptor{ CryptoPP::RSAES_OAEP_SHA_Encryptor enc; };
    struct decryptor{ CryptoPP::RSAES_OAEP_SHA_Decryptor dec; };
    struct rand_gen{ CryptoPP::AutoSeededRandomPool rnd; };
protected:
    std::unique_ptr<prv_key> m_prv_key;
    std::shared_ptr<pub_key> m_pub_key;
    std::shared_ptr<pub_key> m_bobs_pub_key;
    encryptor m_enc;
    decryptor m_dec;
    rand_gen m_rnd;
public:
    enc_eng();
    void set_bobs_pub_key(std::shared_ptr<pub_key> bobs_key);
    std::shared_ptr<const pub_key> get_bobs_pub_key()const;
    std::shared_ptr<const pub_key> get_pub_key()const;
    std::string encrypt(const std::string &mes);
    std::string decrypt(const std::string &mes);
};

};