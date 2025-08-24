#include "BlockChain.h"
#include "Miner.h"
#include "Transaction.h"
#include "Hash.h"
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <unordered_set>

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
    // Convert timestamp to local time with better formatting
    std::time_t raw_time = static_cast<std::time_t>(new_block.time_stamp);
    std::tm* local_time = std::localtime(&raw_time);
    
    std::ostringstream time_stream;
    time_stream << std::put_time(local_time, "%Y-%m-%d %H:%M:%S");
    std::string formatted_time = time_stream.str();
    
    std::string block_hash = new_block.calculate_hash();

    std::cout << "\n+------------------------------------------------------------------+" << std::endl;
    std::cout << "| \U0001f4e6  Block Mined! | Height: " << std::left << std::setw(33) << new_block.block_height << "|" << std::endl;
    std::cout << "+------------------------------------------------------------------+" << std::endl;
    std::cout << "| Timestamp:    " << std::left << std::setw(51) << formatted_time << "|" << std::endl;
    std::cout << "| Transactions: " << std::left << std::setw(51) << new_block.transactions.size() << "|" << std::endl;
    std::cout << "| Nonce:        " << std::left << std::setw(51) << new_block.nonce << "|" << std::endl;
    std::cout << "| Difficulty:   " << std::left << std::setw(51) << new_block.difficulty << "|" << std::endl;
    std::cout << "|                                                                  |" << std::endl;
    std::cout << "| Merkle Root:                                                     |" << std::endl;
    std::cout << "|   " << std::left << std::setw(64) << new_block.merkel_root << "|" << std::endl;
    std::cout << "|                                                                  |" << std::endl;
    std::cout << "| Previous Hash:                                                   |" << std::endl;
    std::cout << "|   " << std::left << std::setw(64) << new_block.pre_hash << "|" << std::endl;
    std::cout << "|                                                                  |" << std::endl;
    std::cout << "| Block Hash:                                                      |" << std::endl;
    std::cout << "|   " << std::left << std::setw(64) << block_hash << "|" << std::endl;
    std::cout << "+------------------------------------------------------------------+" << std::endl;
    std::cout << "| Block Transactions:                                              |" << std::endl;
    
    // Improved transaction display with better formatting
    for (size_t i = 0; i < new_block.transactions.size(); ++i) {
        std::string type = (i == 0) ? " (Coinbase)" : "";
        std::string tx_info = new_block.transactions[i].txid + type;
        
        // Truncate if too long to fit in the display area
        if (tx_info.length() > 58) {
            tx_info = tx_info.substr(0, 55) + "...";
        }
        
        std::cout << "|   - " << std::left << std::setw(58) << tx_info << "|" << std::endl;
    }
    std::cout << "+------------------------------------------------------------------+\n" << std::endl;
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

bool BlockChain::add_transaction_to_pool(const Transaction& tx, std::vector<Transaction>& transaction_pool) {
    // Basic validation of the transaction itself
    if (tx.inputs.empty() || tx.outputs.empty()) {
        return false; // Transaction must have inputs and outputs
    }

    // Coinbase transactions are not added to the pool
    if (tx.inputs[0].index == -1) {
        return false;
    }

    // Get sender's address from the first input's public key
    std::string sender_address = public_key_to_address(tx.inputs[0].public_key);
    
    // 1. Verify the signature
    std::string hash_to_sign = sha256(serialize_2(tx.inputs, tx.outputs));
    if (!verify_signature(tx.inputs[0].public_key, tx.inputs[0].signature, hash_to_sign)) {
        // std::cout << "[Validation Failed] Invalid signature for txid: " << tx.txid << std::endl;
        return false;
    }

    uint64_t total_input_value = 0;
    std::unordered_set<std::string> spent_utxos_in_this_tx;

    // 2. Verify inputs and calculate total input value
    for (const auto& input : tx.inputs) {
        std::string utxo_key = input.txid + ":" + std::to_string(input.index);
        
        // Check if the UTXO exists in the confirmed set
        if (utxo_set.find(utxo_key) == utxo_set.end()) {
            // std::cout << "[Validation Failed] Input UTXO not found for txid: " << tx.txid << std::endl;
            return false; // Input UTXO does not exist
        }

        // Check for double spending within the same transaction
        if (spent_utxos_in_this_tx.count(utxo_key)) {
            //  std::cout << "[Validation Failed] Double spend within same tx: " << tx.txid << std::endl;
            return false;
        }
        spent_utxos_in_this_tx.insert(utxo_key);

        // Ensure the input belongs to the sender
        if (utxo_set.at(utxo_key).address != sender_address) {
            // std::cout << "[Validation Failed] Input UTXO does not belong to sender for txid: " << tx.txid << std::endl;
            return false;
        }
        total_input_value += utxo_set.at(utxo_key).amount;
    }

    // 3. Calculate total output value
    uint64_t total_output_value = 0;
    for (const auto& output : tx.outputs) {
        total_output_value += output.amount;
    }

    // Check if inputs cover outputs
    if (total_input_value < total_output_value) {
        // std::cout << "[Validation Failed] Input value less than output value for txid: " << tx.txid << std::endl;
        return false;
    }

    // 4. Check for double spending against the transaction pool
    for (const auto& pool_tx : transaction_pool) {
        for (const auto& pool_input : pool_tx.inputs) {
            std::string pool_utxo_key = pool_input.txid + ":" + std::to_string(pool_input.index);
            if (spent_utxos_in_this_tx.count(pool_utxo_key)) {
                // std::cout << "[Validation Failed] Input UTXO already in transaction pool for txid: " << tx.txid << std::endl;
                return false;
            }
        }
    }

    // All checks passed, add to the pool
    transaction_pool.push_back(tx);
    return true;
}
