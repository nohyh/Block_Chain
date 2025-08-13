#include"Miningworker.h"

Miningworker::Miningworker(Miner& miner, 
                         BlockChain& blockchain, 
                         std::vector<Transaction> transaction_pool, 
                         std::mutex& pool_mutex, 
                         std::mutex& chain_mutex,
                         std::atomic<bool>& stop_flag)
    : miner_ref(miner),
      blockchain_ref(blockchain),
      transaction_pool_ref(std::move(transaction_pool)),
      pool_mutex_ref(pool_mutex),
      chain_mutex_ref(chain_mutex),
      stop_flag_ref(stop_flag) 
{};

void Miningworker::run(){
  //真正的挖矿逻辑
    
}
