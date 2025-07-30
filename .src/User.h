#ifndef USER_H
#define USER_H

#include<string>
#include <cstddef>      
#include <cstdint>
#include "BlockChain.h"   

class BlockChain;

class User{
public:
    std::string wallet_address;
    Transaction transfer(const std::string &address,const uint64_t &amount,const BlockChain& blockchain)const;
private:
    std::string private_key;
    std::string public_key;
};

#endif