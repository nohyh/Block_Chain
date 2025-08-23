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
    int index;
    std::string signature;
    std::string public_key;
};
class Transaction {
public:
    std::string txid;
    std::vector<Input> inputs;//spent utxo message
    std::vector<Output> outputs; //where is utxo go
    Transaction(std::vector<Input>inputs,std::vector<Output> outputs);
private:
      std::string serialize() const;
};
#endif