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

Block Miner::create_block() {
    size_t block_height = blockchain_ref.blocks.size();
    std::string pre_hash =blockchain_ref.blocks.back().calculate_hash();
    std::string merkel_root =calculate_merkel_root(mempool);
    long long time_stamp =static_cast<long long>(std::time(nullptr));
    uint32_t difficulty=4; // Set difficulty to 4
    uint32_t nonce =0;
    std::vector<Transaction> transactions =mempool;
    Block new_block = Block(block_height,time_stamp,pre_hash,merkel_root,difficulty,nonce,transactions);
    return new_block;
}

Transaction Miner::create_coinbase(){
    std::string zero_hash;
    for(int i=0;i<64;i++){
        zero_hash += "0";
    }
    std::vector<Input>inputs;
    std::vector<Output> outputs;
    outputs.push_back(Output{50 * NOCOIN,this->wallet_address});
    inputs.push_back(Input{zero_hash,-1,"My Mini Blockchain Genesis",""});
    Transaction CoinBase =Transaction(inputs,outputs);
    return CoinBase;
}

bool Miner::verify_transaction(const Transaction &tx) const {
    std::string hash_to_sign =sha256(serialize_2(tx.inputs,tx.outputs));
    if(!verify_signature(tx.inputs[0].public_key,tx.inputs[0].signature,hash_to_sign)){
        return false;
    }
    else{
        return true;
    }
 }