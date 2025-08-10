#include"BlockChain.h"
#include"Miner.h"
#include"Transaction.h"
bool verify_block(const Block& block_to_verify){

};

void add_block(const Block &new_block){

};


BlockChain::BlockChain(const std::string creator_address){
    //该区块前置哈希为16位0
    std::string zero_hash;
    for(int i=0;i<64;i++){
        zero_hash += "0";
    }
    //将创世区块的Transacion数组仅加入一个coinbase奖励
    std::vector<Transaction> transactions;
    std::vector<Input>inputs;
    std::vector<Output> outputs;
    outputs.push_back(Output{50,creator_address});
    inputs.push_back(Input{zero_hash,"-1","My Mini Blockchain Genesis",""});
    Transaction CoinBase =Transaction(inputs,outputs);
    transactions.push_back(CoinBase);
    //创建创世区块
    Block Genesis_Block =Block(0,std::time(nullptr),zero_hash,Miner::calculate_merkel_root(transactions),5,0,transactions);
    //加入utxo集合...
};