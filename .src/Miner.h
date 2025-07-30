#ifndef MINER_H
#define MINER_H

#include"User.h"
#include"Block.h"

class BlockChain;

class Miner : public User{
private:
    BlockChain& blockchain_ref;
    std::vector<Transaction> mempool;
public:
    Miner(const std::string &wallet_address, const std::string &private_key,const std::string &public_key,BlockChain& chain):User(wallet_address,private_key,public_key),blockchain_ref(chain){};
    Block mine();
    Block create_block();
    std::string calculate_merkel_root(std::vector<Transaction> mempool);

};





#endif
