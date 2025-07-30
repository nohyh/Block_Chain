#ifndef TRANSACTION.H
#define TRANSACIOTN.H

#include<User.h>
#include<BlockChain.h>
struct Output{
    uint64_t amount;
    std::string address;
};
struct Input{
    std::string key;//用string key来决定唯一的UTXO，即要花费的UTXO的集合
    std::string signature;
    std::string public_key;
};
class Transaction {
public:
    std::string txid;
    std::vector<Input> inputs;//即花费的UTXO的信息和证明信息
    std::vector<Output> outputs;//即消耗的UTXO的去向
};
#endif