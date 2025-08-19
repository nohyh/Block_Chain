#include"Miningworker.h"

Miningworker::Miningworker(Miner& miner,
                        BlockChain& blockchain,
                        std::vector<Transaction> &transaction_pool, 
                        std::mutex& pool_mutex, 
                        std::mutex& chain_mutex,
                        std::atomic<bool> &stop_flag,
                        std::condition_variable &cv,
                        std::mutex &cv_mutex,
                        bool&block_found_flag
                        )
    : miner_ref(miner),
      blockchain_ref(blockchain),
      transaction_pool_ref(transaction_pool),
      pool_mutex_ref(pool_mutex),
      chain_mutex_ref(chain_mutex),
      stop_flag_ref(stop_flag),
      cv_ref(cv),
      cv_mutex_ref(cv_mutex),
      block_found_flag_ref(block_found_flag)
      

{}

void Miningworker::run() {
  while (true) {
    miner_ref.mempool.push_back(this->miner_ref.create_coinbase());
    //将交易信息拉取到自己的mempool中，感觉这部分逻辑还是有点问题，后面再来优化
    while (miner_ref.mempool.size() < 9) {
      std::unique_lock<std::mutex> lock(pool_mutex_ref);
      if (!transaction_pool_ref.empty()) {
        Transaction tr = transaction_pool_ref.back();
        if (this->miner_ref.verify_transaction(tr)) {
          miner_ref.mempool.push_back(tr);
        }
      }
      //考虑到一秒将会生成一笔交易，所以一秒后肯定至少有一笔交易可以获取，没想到太好的办法
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    //此时mempool已经满了
    //创建新区块 
    Block new_block = this->miner_ref.create_block();
    //下一步，正式开始进行工作量证明
    // 收到提醒前就不会停下
    bool found = false;
    std::string target(new_block.difficulty, '0');
    while (!stop_flag_ref) {
      std::string hash = new_block.calculate_hash();
      if (hash.substr(0, new_block.difficulty) == target && !stop_flag_ref) {
        found = true;
        break;
      }
      new_block.nonce++;
    }
    if (found) {
      std::scoped_lock lock(chain_mutex_ref,pool_mutex_ref);
      if (!stop_flag_ref && blockchain_ref.verify_block(new_block)) {
        blockchain_ref.add_block(new_block);
        blockchain_ref.update_transaction_pool(this->transaction_pool_ref, new_block);
        stop_flag_ref = true;
        //等待一段时间，等所有其他线程都进入下一个循环后再令stop_flag_ref为false
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); 
        stop_flag_ref = false;
      }
    }
    miner_ref.mempool.clear();
  }
}
