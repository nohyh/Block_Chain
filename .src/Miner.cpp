#include "Miner.h"
#include "BlockChain.h"
#include <ctime>
std::string Miner::calculate_merkel_root(const std::vector<Transaction> &mempool){
    std::vector<std::string> hash_list;
    for(const auto &transaction:mempool){
        hash_list.push_back(transaction.txid);
    }
    while(hash_list.size()>1){
        std::vector<std::string> next_list;
        for (size_t i = 0; i < hash_list.size(); i += 2) {
            if (i + 1 < hash_list.size()) {
                next_list.push_back(sha256(hash_list[i] + hash_list[i + 1]));
            } 
            else {
                next_list.push_back(sha256(hash_list[i] + hash_list[i]));
            }
        }
        hash_list =next_list;
    }
    return hash_list[0];
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
  
    
}
