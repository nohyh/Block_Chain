#ifndef MINER_H
#define MINER_H

#include"User.h"

class BlockChain;

class Miner : public User{
private:
    std::vector<Transaction> mempool;
public:
    Miner(const std::string &wallet_address3, const std::string &private_key,const std::string &public_key,BlockChain& chain):User(wallet_address3,private_key,public_key,chain){};
    Block mine();
    Block create_block();
    static std::string calculate_merkel_root(const std::vector<Transaction> &mempool);

};





#endif
