#include<iostream>
#include<sstream>
#include<algorithm>
#include <string>
#include"Hash.h"
#include "Block.h"
#include"Transform.h"

Block::Block(size_t block_height,long long stamp,std::string prehash,std::string merkel_root,uint32_t difficulty,uint32_t nonce,std::vector<Transaction> transactions){
    this->block_height = block_height;
    this->time_stamp = stamp;
    this->pre_hash = prehash;
    this->merkel_root = merkel_root;
    this->difficulty = difficulty;
    this->nonce = nonce; 
    this->transactions = transactions;
};
//前块哈希 -> 默克尔根 -> 时间戳 -> 难度 -> Nonce
//时间戳, 难度, Nonce 为整数，pre_hash和merkel_root为16进制字符串

std::string Block::calculate_hash()const{
    std::stringstream ss ;
    std::string pre_Hash_binary =convert_16_2(pre_hash);
    std::reverse(pre_Hash_binary.begin(),pre_Hash_binary.end());
    ss.write(pre_Hash_binary.data(),32);
    std::string merkel_root_binary =convert_16_2(merkel_root);
    std::reverse(merkel_root_binary.begin(),merkel_root_binary.end());
    ss.write(merkel_root_binary.data(),32);
    ss.write(reinterpret_cast<const char*>(&time_stamp),sizeof(time_stamp));
    ss.write(reinterpret_cast<const char*>(&difficulty),sizeof(difficulty));
    ss.write(reinterpret_cast<const char*>(&nonce),sizeof(nonce));
    std::string header_data =ss.str();
    std::string first_hash_result =sha256(header_data);
    std::string final_binary_hash =sha256(first_hash_result);
    std::string result =convert_2_16(final_binary_hash);
    return result;
}