#ifdef BLOCKCHAIN.H
#define BLOCKCHIAN.H

class UTXO{
public:
    uint64_t amount;
    int index;
    std::string address;
    std::string TXID;
    std::string key;
    std::string get_utxo_key() const{
        return this->txid + ":" + std::to_string(this->index);
    }
    UTXO(uint64_t amount,int index,std::string address,std::string TXID);
}

class BlockChain{
public:
    std::unordered_map<std::string,UTXO> utxo_set;
    std::vector<Block> block_chain;
    void verify_block//检测矿工提交的块是否合格
    void add_block();
    
    
}


#endif 