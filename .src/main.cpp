#include <iostream>
#include<random>
#include "BlockChain.h"
#include "Miner.h"
#include "Hash.h"
#include"Miningworker.h"
int main() {
    std::cout << "Blockchain system starting..." << std::endl;
    std::vector<Transaction> Transaction_pool;
    std::mutex pool_mtx;
    std::mutex chain_mtx;
    std::atomic<bool> stop_flag;
    std::condition_variable cv;
    std::mutex cv_mtx;
    bool block_found;
    struct KeyPair my_keys =generate_keys();
    BlockChain blockchain = BlockChain(my_keys.wallet_address_hex);
    //将接受了奖励的地址和nohyh绑定
    User nohyh =User(my_keys.wallet_address_hex,my_keys.private_key_bin,my_keys.public_key_bin,blockchain);
   //创建其他矿工
    std::vector<Miner> miners;
    std::vector<Miningworker> workers;
    std::vector<std::thread> thread_miners;
    //创建矿工并加入矿工数组
    for(int i=0;i<10;i++){
        KeyPair keys =generate_keys();
        miners.emplace_back(keys.wallet_address_hex,keys.private_key_bin,keys.public_key_bin,blockchain);
    }
    //创建工作矿工并加入工作矿工数组并开始执行
    for(auto &miner:miners){
        workers.emplace_back(miner,blockchain,Transaction_pool,pool_mtx,chain_mtx,stop_flag,cv,cv_mtx,block_found);
    }
    for(auto &worker :workers){
        thread_miners.emplace_back([worker_copy = worker]()mutable{
            worker_copy.run();
        });
    }
    //同样的，创建用户并开始交易：
    std::vector<User> users;
    users.push_back(nohyh);
    for(int i=0;i<49;i++){
        KeyPair keys =generate_keys();
        users.emplace_back(keys.wallet_address_hex,keys.private_key_bin,keys.public_key_bin,blockchain);
    }
    std::thread trading_thread(auto_trade, std::ref(users), std::ref(Transaction_pool), std::ref(pool_mtx), std::ref(blockchain));
    //等待线程完成后再继续（实际上不会继续了）
    trading_thread.join();
    for(auto &t:thread_miners){
        t.join();
    }

}

void auto_trade(std::vector<User>& users,std::vector<Transaction>& transaction_pool,std::mutex& pool_mutex,BlockChain& blockchain){
    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::vector<int> indices ; //此为有储存有钱用户的下标的数组
    indices.push_back(0);
    
    while(true){
        //首先从有钱的人中间找到付款方
        std::uniform_int_distribution<> dist(0,indices.size()-1);
        int sender_index =indices[dist(gen)];
        User& sender = users[sender_index];
        uint64_t balance=blockchain.get_balance(sender.wallet_address);
        if(balance==0) continue;
        //然后找到收款方
        std::uniform_int_distribution<> receiver_dist(0,users.size()-1);
        int receiver_index =receiver_dist(gen);
        User &receiver =users[receiver_index];
        if(receiver.wallet_address==sender.wallet_address) continue;
        //确认金额
        std::uniform_int_distribution<> amount_dist(1,balance);
        int amount =amount_dist(gen);
        //开始转账
        try {
            Transaction tr = sender.transfer(receiver.wallet_address, amount);
            {
            std::lock_guard<std::mutex> lock(pool_mutex);
            transaction_pool.push_back(tr);
            }
            if (std::find(indices.begin(), indices.end(), receiver_index) == indices.end()) {
            indices.push_back(receiver_index);
            }
        }
        catch (const std::exception &e) {
        }
        std::cout<<sender.wallet_address<<"Transfer "<<amount/NOCOIN<<"To "<<receiver.wallet_address<<std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}