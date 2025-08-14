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
  miner_ref.mempool.push_back(this->miner_ref.create_coinbase());
  //将交易信息拉取到自己的mempool中，感觉这部分逻辑还是有点问题
  while(miner_ref.mempool.size()<9){
    std::lock_guard<std::mutex> lock(pool_mutex_ref);
    if(!transaction_pool_ref.empty()){
      Transaction tr =transaction_pool_ref.back();
      if(this->miner_ref.verify_transaction(tr)){
        miner_ref.mempool.push_back(tr);
      }
    }
      //考虑到一秒将会生成一笔交易，所以一秒后肯定至少有一笔交易可以获取，没想到太好的办法
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  //此时mempool已经满了
  //创建新区块 
  Block new_block =this->miner_ref.create_block();
  //下一步，正式开始进行工作量证明
  

}
