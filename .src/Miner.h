#ifndef MINER_H
#define MINER_H

#include"User.h"

class BlockChain;

class Miner : public User{
private:
    std::vector<Transaction> mempool;
     std::string calculate_merkel_root(const std::vector<Transaction> &mempool);
public:
    Miner(const std::string &wallet_address, const std::string &private_key,const std::string &public_key,BlockChain& chain):User(wallet_address,private_key,public_key,chain){};
    Block mine();
    Block create_block();

};





#endif
