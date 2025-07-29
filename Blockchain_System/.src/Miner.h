#ifndef MINER_H
#define MINER_H

#include<User.h>

class Miner : public User{
public:
    Block mine(const BlockChain& blockchain);

private:
    std::vector<Transaction> mempool;
};






#endif
