#ifndef BLOCK_H
#define BLOCK_H

#include <cstddef>      
#include <cstdint>     
#include <string>      
#include <vector>          
#include "Transaction.h" 
#include <utility> 
class Block{
public:
    size_t block_height;
    long long time_stamp;
    std::string pre_hash;
    std::string merkel_root;
    size_t difficulty;
    uint32_t nouce;
    std::vector<Transaction> transactions;
    Block(long long stamp,std::string prehash,std::string mekerl_root,size_t difficulty,uint32_t nouce,std::vector<Transaction> transactions);//应该在数组满后进行对象的初始化，需要加入已知的参数
    std::string calculate_hash()const;
    bool is_valid();
};
#endif