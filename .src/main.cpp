#include <iostream>
#include<random>
#include <algorithm>
#include "BlockChain.h"
#include "Miner.h"
#include "Hash.h"
#include"Miningworker.h"
#include "Transform.h"

// Forward declaration with the new signature including chain_mutex
void auto_trade(std::vector<User>& users, std::vector<Transaction>& transaction_pool, std::mutex& pool_mutex, std::mutex& chain_mutex, BlockChain& blockchain, std::condition_variable& cv);

int main() {
    std::cout << "Blockchain system starting..." << std::endl;
    std::vector<Transaction> Transaction_pool;
    std::mutex pool_mtx;
    std::mutex chain_mtx;
    std::atomic<bool> stop_flag = false;
    std::condition_variable cv;
    std::mutex cv_mtx;
    bool block_found = false; // This seems unused now, can be removed later.

    struct KeyPair my_keys =generate_keys();
    BlockChain blockchain = BlockChain(my_keys.wallet_address_hex);
    // Bind the address that received the reward to nohyh
    User nohyh =User(my_keys.wallet_address_hex,my_keys.private_key_bin,my_keys.public_key_bin,blockchain);

   // Create other miners
    std::vector<Miner> miners;
    std::vector<Miningworker> workers;
    std::vector<std::thread> thread_miners;
    // Create miners and add them to the miners vector
    for(int i=0;i<10;i++){
        KeyPair keys =generate_keys();
        miners.emplace_back(keys.wallet_address_hex,keys.private_key_bin,keys.public_key_bin,blockchain);
    }

    // Create worker miners, add them to the workers vector, and start execution
    for(auto &miner:miners){
        workers.emplace_back(miner,blockchain,Transaction_pool,pool_mtx,chain_mtx,stop_flag,cv,cv_mtx,block_found);
    }
    for(auto &worker :workers){
        thread_miners.emplace_back([&worker](){
            worker.run();
        });
    }

    // Similarly, create users and start trading:
    std::vector<User> users;
    users.push_back(nohyh);
    for(int i=0;i<49;i++){
        KeyPair keys =generate_keys();
        users.emplace_back(keys.wallet_address_hex,keys.private_key_bin,keys.public_key_bin,blockchain);
    }

    // Pass chain_mtx to the trading_thread
    std::thread trading_thread(auto_trade, std::ref(users), std::ref(Transaction_pool), std::ref(pool_mtx), std::ref(chain_mtx), std::ref(blockchain), std::ref(cv));
    
    // Wait for the threads to complete before continuing (in practice, it will loop forever)
    trading_thread.join();
    for(auto &t:thread_miners){
        t.join();
    }

}

// auto_trade implementation with the new signature
void auto_trade(std::vector<User>& users, std::vector<Transaction>& transaction_pool, std::mutex& pool_mutex, std::mutex& chain_mutex, BlockChain& blockchain, std::condition_variable& cv){
    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::vector<int> indices ; // This is an array of indices for users who have money
    indices.push_back(0);
    
    while(true){
        {
            // Lock chain first, then pool, to prevent deadlock and data races on utxo_set
            std::lock_guard<std::mutex> chain_lock(chain_mutex);
            std::lock_guard<std::mutex> pool_lock(pool_mutex);

            std::uniform_int_distribution<> dist(0,indices.size()-1);
            int sender_index =indices[dist(gen)];
            User& sender = users[sender_index];
            
            uint64_t balance = blockchain.get_balance(sender.wallet_address);
            if(balance > 0) {
                std::uniform_int_distribution<> receiver_dist(0,users.size()-1);
                int receiver_index = receiver_dist(gen);
                User &receiver = users[receiver_index];

                if(receiver.wallet_address != sender.wallet_address) {
                    std::uniform_int_distribution<uint64_t> amount_dist(1,balance);
                    uint64_t amount = amount_dist(gen);

                    try {
                        Transaction tr = sender.transfer(receiver.wallet_address, amount);
                        bool success = blockchain.add_transaction_to_pool(tr, transaction_pool);

                        if (success) {
                            cv.notify_all();
                            if (std::find(indices.begin(), indices.end(), receiver_index) == indices.end()) {
                                indices.push_back(receiver_index);
                            }
                            std::cout << "\n[+] New Valid Transaction Added to Pool" << std::endl;
                            //std::cout << "    - Sender:   " << sender.wallet_address << std::endl;
                            //std::cout << "    - Receiver: " << receiver.wallet_address << std::endl;
                            //std::cout << "    - Amount:   " << format_amount(amount) << " NOCOIN" << std::endl;
                            std::cout<<sender.wallet_address<<"  send  "<<format_amount(amount) <<" NOCOIN  TO   "<<receiver.wallet_address<<std::endl;
                        } 
                    }
                    catch (const std::exception &e) {
                        // This case should be rare now but is kept for safety
                    }
                }
            }
        } // All locks are released here

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}