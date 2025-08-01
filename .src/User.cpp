#include "User.h"
bool hex_verify(const std::string &address){

}

Transaction User::transfer(const std::string &address,const uint64_t &amount)const{
    if( address.length()!=64||!hex_verify(address)||amount<=0){
        throw std::invalid_argument("Invalid input");
    }
    std::vector<UTXO>my_utxos;
    for(auto utxo:blockchain_ref.utxo_set){
        if(utxo.second.address==this->wallet_address){
            my_utxos.push_back(utxo.second);//获取UTXO
        }
    }

};