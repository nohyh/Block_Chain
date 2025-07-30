#ifndef USER_H
#define USER_H

#include<string>
#include <cstddef>      
#include <cstdint> 
#include"Transaction.h"

class BlockChain;

class User{
public:
    std::string wallet_address;
    User(const std::string &wallet_address, const std::string &private_key,const std::string &public_key) :wallet_address(wallet_address),private_key(private_key),public_key(public_key){};
    Transaction transfer(const std::string &address,const uint64_t &amount,const BlockChain& blockchain)const;
private:
    std::string private_key;
    std::string public_key;
};

#endif