#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include<Block.h>
#include <unordered_map>

class UTXO{
public:
    uint64_t amount;
    int index;
    std::string address;
    std::string txid;
    std::string get_utxo_key() const{
        return this->txid + ":" + std::to_string(this->index);
    }
    UTXO(uint64_t amount,int index,std::string address,std::string txid);
};

class BlockChain{
public:
    std::unordered_map<std::string,UTXO> utxo_set;
    std::vector<Block> blocks;
    bool verify_block(const Block& block_to_verify)const;//检测矿工提交的块是否合格
    void add_block(const Block &new_block);
    BlockChain();//创建创世区块并开始进行模拟
    
    
};


#endif 