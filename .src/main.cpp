#include <iostream>
#include "BlockChain.h"
#include "Miner.h"
#include "Hash.h"
int main() {
    std::cout << "Blockchain system starting..." << std::endl;
    struct KeyPair my_keys =generate_keys();
    BlockChain blockchain = BlockChain(my_keys.wallet_address_hex);
    Miner nohyh =Miner(my_keys.wallet_address_hex,my_keys.private_key_bin,my_keys.public_key_bin,blockchain);
    
    return 0;
}
