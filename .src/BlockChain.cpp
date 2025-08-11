#include"BlockChain.h"
#include"Miner.h"
#include"Transaction.h"
#include"Hash.h"
bool BlockChain::verify_block(const Block& block_to_verify)const{
    int n=0;
    std::unordered_set<std::string> spent_utxo;
    //检查pre_hash是否一致
    if(block_to_verify.pre_hash!=this->blocks.back().calculate_hash()){
        return false;
    }
    //验证工作量证明，算出来的哈希值应当符合难度要求
    for(auto ch:block_to_verify.calculate_hash()){
        if(ch!='0'){
            break;
        }
        n++;
    }
    if(n!=block_to_verify.difficulty){
        return false;
    }
    //检查区块中的每一笔交易
    for(int i=0;i<block_to_verify.transactions.size();i++){
        //检查coinbase交易
        if(i==0){
            if(block_to_verify.transactions[0].inputs.size()!=1||block_to_verify.transactions[0].inputs[0].index!=-1||block_to_verify.transactions[0].outputs[0].amount!=50){
                return false;
            }
            continue;
        }
        //正常的交易
        for(auto input:block_to_verify.transactions[i].inputs){
            //双花攻击检查(判断该utxo是否还存在，以及该utxo是否已经在块中被使用)
            std::string key =input.txid + ":" + std::to_string(input.index);
            if(utxo_set.count(key)==0||spent_utxo.count(key)!=0){
                return false;
            }
            //检查公钥是否与地址匹配
            auto owner_address =this->utxo_set.at(key).address;
            if(public_key_to_address(input.public_key)!=owner_address){
                return false;
            }
            //进行签名验证
            std::string hash_to_sign =sha256(serialize_2(block_to_verify.transactions[i].inputs,block_to_verify.transactions[i].outputs));
            if(!verify_signature(input.public_key,input.signature,hash_to_sign)){
                return false;
            }
            //记录该uxto，用作下一步的对比验证
            spent_utxo.insert(key);
        }
    }
    return true;
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
    inputs.push_back(Input{zero_hash,-1,"My Mini Blockchain Genesis",""});
    Transaction CoinBase =Transaction(inputs,outputs);
    transactions.push_back(CoinBase);
    //创建创世区块
    Block Genesis_Block =Block(0,std::time(nullptr),zero_hash,Miner::calculate_merkel_root(transactions),5,0,transactions);
    //加入utxo集合和blocks
    blocks.push_back(Genesis_Block);
    UTXO new_utxo =UTXO(outputs[0].amount,0,outputs[0].address,CoinBase.txid);
    std::string key =new_utxo.get_utxo_key();
    utxo_set[key]=new_utxo;
};