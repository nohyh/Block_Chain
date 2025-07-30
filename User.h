#ifndef USER_H
#define USER_H

#include<Block.h>
class User{
public:
    std::string wallet_address;
private:
    std::string private_key;
    std::string public_key;
    void transfer(std::string address,uint64_t amount);
};

#endif