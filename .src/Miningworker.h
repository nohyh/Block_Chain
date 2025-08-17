#ifndef MINING_WORKER_H
#define MINING_WORKER_H

#include"Miner.h"

class Miningworker{
public:
    Miningworker(Miner& miner, 
                 BlockChain& blockchain, 
                 std::vector<Transaction> &transaction_pool, 
                 std::mutex& pool_mutex, 
                 std::mutex& chain_mutex,
                 std::atomic<bool>& stop_flag,
                std::condition_variable& cv,
                 std::mutex& cv_mutex,
                 bool& block_found_flag
                );
    void run();
private:
    Miner& miner_ref;
    BlockChain& blockchain_ref;
    std::vector<Transaction>& transaction_pool_ref;
    std::mutex& pool_mutex_ref;
    std::mutex& chain_mutex_ref;
    std::atomic<bool>& stop_flag_ref;
    std::condition_variable& cv_ref;
    std::mutex& cv_mutex_ref;
    bool& block_found_flag_ref;


};
#endif // MINING_WORKER_H