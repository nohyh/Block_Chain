#ifndef USER_H
#define USER_H

#include<string>
#include <cstddef>      
#include <cstdint> 
#include"Transaction.h"
#include"Block.h"
#include"Transform.h"
#include <stdexcept>
#include "BlockChain.h"

#include <openssl/ec.h>      // 用于椭圆曲线加密 (EC_KEY, EC_GROUP)
#include <openssl/ecdsa.h>   // 用于ECDSA签名 (ECDSA_do_sign, ECDSA_SIG)
#include <openssl/obj_mac.h> // 包含了曲线的ID (NID_secp256k1)
#include <openssl/bn.h>  

class BlockChain;

class User{
public:
    std::string wallet_address;
    BlockChain& blockchain_ref;//每个用户都自存区块链的引用，这样在方法里就可以调用自身的区块链成员来获取相关成员而不用每次都加入参数
    User(const std::string &wallet_address, const std::string &private_key,const std::string &public_key,BlockChain& chain) :wallet_address(wallet_address),private_key(private_key),public_key(public_key),blockchain_ref(chain){};
    Transaction transfer(const std::string &address,const uint64_t &amount)const;
private:
    std::string private_key;
    std::string public_key;
    std::string sign(const std::string &hash_to_sign) const;
};

std::string serialize_2(std::vector<Input> inputs,std::vector<Output> outputs);//对之前的serialize()略加修改即可
#endif