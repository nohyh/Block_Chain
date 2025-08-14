#ifndef MINER_H
#define MINER_H

#include"User.h"
#include<mutex>
#include<thread>

class BlockChain;

class Miner : public User{
private:
public:
    std::vector<Transaction> mempool;
    Miner(const std::string &wallet_address3, const std::string &private_key,const std::string &public_key,BlockChain& chain):User(wallet_address3,private_key,public_key,chain){};
    Block create_block();
    static std::string calculate_merkel_root(const std::vector<Transaction> &mempool);
    bool verify_transaction(const Transaction &tx)const ;
    Transaction create_coinbase();
};





#endif
