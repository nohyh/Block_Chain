
#ifndef HASH_H
#define HASH_H

#include <string>

std::string sha256(const std::string& str);
std::string ripemd160(const std::string& str);

struct KeyPair {
    std::string private_key_bin; // 32byte
    std::string public_key_bin;  // 65byte
    std::string wallet_address_hex; // 64byte
};

KeyPair generate_keys();

std::string public_key_to_address(const std::string& public_key_bin);

bool verify_signature(const std::string& public_key_bin, 
                      const std::string& signature_der, 
                      const std::string& hash_to_sign);

#endif // HASH_H
