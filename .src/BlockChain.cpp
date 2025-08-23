#include "BlockChain.h"
#include "Miner.h"
#include "Transaction.h"
#include "Hash.h"
#include <chrono>
#include <algorithm>

UTXO::UTXO(uint64_t amount,int index,std::string address,std::string txid):amount(amount),index(index),address(address),txid(txid){}

void BlockChain::update_transaction_pool(std::vector<Transaction>& pool, const Block& new_block) {
    if (pool.empty()) {
        return;
    }
    std::unordered_set<std::string> confirmed_txids;
    for (const auto& tr : new_block.transactions) {
        confirmed_txids.insert(tr.txid);
    }
    pool.erase(std::remove_if(pool.begin(), pool.end(),
        [&](const Transaction& tx) {
            return confirmed_txids.count(tx.txid) > 0;
        }
    ), pool.end());
}

bool BlockChain::verify_block(const Block& block_to_verify)const {
    int n = 0;
    std::unordered_set<std::string> spent_utxo;
    // Check if pre_hash is consistent
    if (block_to_verify.pre_hash != this->blocks.back().calculate_hash()) {
        return false;
    }
    // Verify the proof of work, the calculated hash should meet the difficulty requirement
    for (auto ch : block_to_verify.calculate_hash()) {
        if (ch != '0') {
            break;
        }
        n++;
    }
    if (n < block_to_verify.difficulty) { // Fix: Should be >= difficulty
        return false;
    }
    // Check every transaction in the block
    for (size_t i = 0; i < block_to_verify.transactions.size(); i++) {
        // Check the coinbase transaction
        if (i == 0) {
            if (block_to_verify.transactions[0].inputs.size() != 1 || block_to_verify.transactions[0].inputs[0].index != -1 || block_to_verify.transactions[0].outputs[0].amount != 50 * NOCOIN) {
                return false;
            }
            continue;
        }
        // Normal transactions
        for (const auto& input : block_to_verify.transactions[i].inputs) {
            // Double-spend check (check if the utxo still exists and has not been spent in this block)
            std::string key = input.txid + ":" + std::to_string(input.index);
            if (utxo_set.count(key) == 0 || spent_utxo.count(key) != 0) {
                return false;
            }
            // Check if the public key matches the address
            auto owner_address = this->utxo_set.at(key).address;
            if (public_key_to_address(input.public_key) != owner_address) {
                return false;
            }
            // Perform signature verification
            std::string hash_to_sign = sha256(serialize_2(block_to_verify.transactions[i].inputs, block_to_verify.transactions[i].outputs));
            if (!verify_signature(input.public_key, input.signature, hash_to_sign)) {
                return false;
            }
            // Record this utxo for subsequent verification
            spent_utxo.insert(key);
        }
    }
    return true;
}

void BlockChain::print_block(const Block& new_block) const {
    auto tp = std::chrono::system_clock::from_time_t(new_block.time_stamp);
    std::cout << "===========Block=============" << std::endl;
    std::cout << "Time= " << std::format("{:%Y-%m-%d %H:%M:%S}", tp) << std::endl;
    std::cout << "Height= " << new_block.block_height << std::endl;
    std::cout << "Hash= " << new_block.calculate_hash() << std::endl;
    std::cout << "Nonce= " << new_block.nonce << std::endl;
    std::cout << "==============================" << std::endl;
}
void BlockChain::add_block(const Block &new_block){
    // Adding this block to the vector means adding it to the chain
    this->blocks.push_back(new_block);
    // Update the UTXO set, delete old UTXOs, and create new ones
    for(size_t i=0;i<new_block.transactions.size();i++){
        // Process the first transaction (coinbase)
        if(i==0){
            for(const auto& output:new_block.transactions[i].outputs){
                UTXO new_utxo =UTXO(output.amount,0,output.address,new_block.transactions[i].txid);
                utxo_set.emplace(new_utxo.get_utxo_key(), new_utxo);
            }
        }
        // Normal transactions
        else{
            for(const auto& input: new_block.transactions[i].inputs){
                std::string key =input.txid + ":" + std::to_string(input.index);
                utxo_set.erase(key);
            }
            int j=0;
             for(const auto& output:new_block.transactions[i].outputs){
                UTXO new_utxo =UTXO(output.amount,j,output.address,new_block.transactions[i].txid);
                utxo_set.emplace(new_utxo.get_utxo_key(), new_utxo);
                j++;
            }
        }
    }
    print_block(new_block);
}


BlockChain::BlockChain(const std::string creator_address){
    // The previous hash of this block is 64 zeros
    std::string zero_hash(64, '0');

    // Add only one coinbase reward to the transaction array of the genesis block
    std::vector<Transaction> transactions;
    std::vector<Input>inputs;
    std::vector<Output> outputs;
    outputs.push_back(Output{50 * NOCOIN,creator_address});
    inputs.push_back(Input{zero_hash,-1,"My Mini Blockchain Genesis",""});
    Transaction CoinBase =Transaction(inputs,outputs);
    transactions.push_back(CoinBase);

    // Create the genesis block
    Block Genesis_Block =Block(0,std::time(nullptr),zero_hash,Miner::calculate_merkel_root(transactions),5,0,transactions);
    
    // Add to the utxo set and blocks vector
    blocks.push_back(Genesis_Block);
    UTXO new_utxo =UTXO(outputs[0].amount,0,outputs[0].address,CoinBase.txid);
    std::string key =new_utxo.get_utxo_key();
    utxo_set.emplace(key, new_utxo);
}

uint64_t BlockChain::get_balance(const std::string &address)const{
    uint64_t deposit=0;
    for(const auto &utxo:utxo_set){
        if(utxo.second.address==address){
            deposit+=utxo.second.amount;
        }
    }
    return deposit;
}