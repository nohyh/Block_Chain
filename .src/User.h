#ifndef USER_H
#define USER_H

#include<string>
#include <cstddef>      
#include <cstdint> 
#include"Transaction.h"
#include"Block.h"
#include"Transform.h"
#include <stdexcept>
#include "BlockChain.h"

#include <openssl/ec.h>      
#include <openssl/ecdsa.h>   
#include <openssl/obj_mac.h> 
#include <openssl/bn.h>  

class BlockChain;

class User{
public:
    std::string wallet_address;
    BlockChain& blockchain_ref;
    User(const std::string &wallet_address, const std::string &private_key,const std::string &public_key,BlockChain& chain) :wallet_address(wallet_address),private_key(private_key),public_key(public_key),blockchain_ref(chain){};
    Transaction transfer(const std::string &address,const uint64_t &amount)const;
private:
    std::string private_key;
    std::string public_key;
    std::string sign(const std::string &hash_to_sign) const;
};

std::string serialize_2(const std::vector<Input> &inputs,const std::vector<Output> &outputs);
#endif