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
        miner_ref.mempool.clear();
        miner_ref.mempool.push_back(miner_ref.create_coinbase());

        {
            std::unique_lock<std::mutex> lock(pool_mutex_ref);
            cv_ref.wait(lock, [this] { return transaction_pool_ref.size() >= 8; });

            // Take a snapshot of the transactions to avoid holding the lock for too long
            std::vector<Transaction> current_pool_snapshot = transaction_pool_ref;
            lock.unlock();

            for (const auto& tx : current_pool_snapshot) {
                if (miner_ref.mempool.size() >= 9) break;
                // Simple check to avoid duplicates in the mempool
                bool found = false;
                for (const auto& mem_tx : miner_ref.mempool) {
                    if (mem_tx.txid == tx.txid) {
                        found = true;
                        break;
                    }
                }
                if (!found && miner_ref.verify_transaction(tx)) {
                    miner_ref.mempool.push_back(tx);
                }
            }
        }

        Block new_block = miner_ref.create_block();
        size_t initial_chain_height = 0;
        {
            std::lock_guard<std::mutex> chain_lock(chain_mutex_ref);
            initial_chain_height = blockchain_ref.blocks.size();
        }

        std::string target(new_block.difficulty, '0');
        bool found_by_me = false;

        while (true) {
            if (new_block.calculate_hash().substr(0, new_block.difficulty) == target) {
                found_by_me = true;
                break;
            }

            new_block.nonce++;

            if (new_block.nonce % 1000 == 0) {
                std::lock_guard<std::mutex> chain_lock(chain_mutex_ref);
                if (blockchain_ref.blocks.size() > initial_chain_height) {
                    break; // Another miner won
                }
            }
        }

        if (found_by_me) {
            std::lock_guard<std::mutex> lock(chain_mutex_ref);
            if (blockchain_ref.blocks.size() == initial_chain_height) {
                if (blockchain_ref.verify_block(new_block)) {
                    blockchain_ref.add_block(new_block);
                    {
                        std::lock_guard<std::mutex> pool_lock(pool_mutex_ref);
                        blockchain_ref.update_transaction_pool(transaction_pool_ref,new_block);
                    }
                    cv_ref.notify_all();
                }
            }
        }
    }
}
