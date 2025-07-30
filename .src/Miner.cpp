#include "Miner.h"
#include "BlockChain.h"
#include <ctime>
std::string Miner::calculate_merkel_root(std::vector<Transaction> mempool){
    //明天再写
};

Block Miner::create_block(){
    size_t block_height = blockchain_ref.blocks.size();
    std::string pre_hash =blockchain_ref.blocks.back().calculate_hash();
    std::string merkel_root =calculate_merkel_root(mempool);
    long long time_stamp =static_cast<long long>(std::time(nullptr));
    uint32_t difficulty=0;//以后再设定
    uint32_t nonce =0;
    std::vector<Transaction> transactions =mempool;
    Block new_block = Block(block_height,time_stamp,pre_hash,merkel_root,difficulty,nonce,transactions);
    return new_block;
}

Block Miner::mine(){
    //实现挖矿逻辑，返回上链的区块
}
