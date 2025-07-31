#ifndef TRANSACTION_H
#define TRANSACTION_H

#include<vector>
#include<string>
#include<sstream>
#include<Transform.h>
#include "Hash.h" 

struct Output{
    uint64_t amount;
    std::string address;
};
struct Input{
    std::string txid;
    std::string index;
    std::string signature;
    std::string public_key;
};
class Transaction {
public:
    std::string txid;
    std::vector<Input> inputs;//即花费的UTXO的信息和证明信息
    std::vector<Output> outputs;//即消耗的UTXO的去向
    Transaction(std::vector<Input>inputs,std::vector<Output> outputs);
private:
      std::string serialize() const;
};
#endif