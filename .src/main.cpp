#include <iostream>
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
    struct KeyPair my_keys =generate_keys();
    BlockChain blockchain = BlockChain(my_keys.wallet_address_hex);
    //将接受了奖励的地址和nohyh绑定
    Miner nohyh =Miner(my_keys.wallet_address_hex,my_keys.private_key_bin,my_keys.public_key_bin,blockchain);
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
        workers.emplace_back(miner,blockchain,Transaction_pool,pool_mtx,chain_mtx,stop_flag);
    }
    for(auto &worker :workers){
        thread_miners.emplace_back([worker_copy = worker]()mutable{
            worker_copy.run();
        });
    }
}
