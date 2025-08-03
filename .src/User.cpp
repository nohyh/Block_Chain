#include "User.h"
#include "Hash.h"
#include"Transaction.h"
    // 用于大数运算 (BIGNUM)
std::string User::sign(const std::string& hash_to_sign) const {
    // --- 步骤 1: 将我们的私钥加载到OpenSSL的EC_KEY结构中 ---

    // 1a. 创建一个 BIGNUM 对象来存储我们的二进制私钥
    BIGNUM* bn_private_key = BN_bin2bn(
        reinterpret_cast<const unsigned char*>(this->private_key.c_str()),
        this->private_key.length(),
        nullptr // BIGNUM对象由函数创建并返回
    );
    if (bn_private_key == nullptr) {
        throw std::runtime_error("Failed to convert private key to BIGNUM.");
    }

    // 1b. 创建一个 EC_KEY 对象，并指定使用 secp256k1 曲线
    EC_KEY* ec_key = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (ec_key == nullptr) {
        BN_free(bn_private_key); // 清理内存
        throw std::runtime_error("Failed to create new EC_KEY.");
    }

    // 1c. 将 BIGNUM 形式的私钥设置到 EC_KEY 对象中
    if (EC_KEY_set_private_key(ec_key, bn_private_key) != 1) {
        BN_free(bn_private_key);
        EC_KEY_free(ec_key);
        throw std::runtime_error("Failed to set private key.");
    }

    // 1d. 让OpenSSL根据私钥自动计算出对应的公钥
    // 这是必需的，因为签名算法内部需要用到曲线参数，这些参数在设置公钥时被完全初始化
    const EC_GROUP* group = EC_KEY_get0_group(ec_key);
    EC_POINT* pub_point = EC_POINT_new(group);
    if (EC_POINT_mul(group, pub_point, bn_private_key, nullptr, nullptr, nullptr) != 1) {
        BN_free(bn_private_key);
        EC_KEY_free(ec_key);
        EC_POINT_free(pub_point);
        throw std::runtime_error("Failed to calculate public key point.");
    }
    if (EC_KEY_set_public_key(ec_key, pub_point) != 1) {
        BN_free(bn_private_key);
        EC_KEY_free(ec_key);
        EC_POINT_free(pub_point);
        throw std::runtime_error("Failed to set public key.");
    }

    // --- 步骤 2: 执行签名 ---

    // ECDSA_do_sign 会对传入的哈希（hash_to_sign）进行签名
    // 结果是一个 ECDSA_SIG 结构体，其中包含 r 和 s 两个大数值
    ECDSA_SIG* signature = ECDSA_do_sign(
        reinterpret_cast<const unsigned char*>(hash_to_sign.c_str()),
        hash_to_sign.length(),
        ec_key
    );
    if (signature == nullptr) {
        BN_free(bn_private_key);
        EC_KEY_free(ec_key);
        EC_POINT_free(pub_point);
        throw std::runtime_error("Failed to generate ECDSA signature.");
    }


    // --- 步骤 3: 将签名序列化为DER编码的字符串 ---

    // 获取DER编码后签名所需的最大缓冲区大小
    int der_sig_len = i2d_ECDSA_SIG(signature, nullptr);
    if (der_sig_len <= 0) {
        BN_free(bn_private_key);
        EC_KEY_free(ec_key);
        EC_POINT_free(pub_point);
        ECDSA_SIG_free(signature);
        throw std::runtime_error("Failed to determine DER signature length.");
    }

    // 创建一个足够大的vector来存放DER编码的签名
    std::vector<unsigned char> der_signature(der_sig_len);
    unsigned char* p = der_signature.data(); // 获取vector缓冲区的裸指针

    // i2d_ECDSA_SIG 再次调用，这次它会将编码后的数据写入我们提供的缓冲区
    if (i2d_ECDSA_SIG(signature, &p) <= 0) {
        BN_free(bn_private_key);
        EC_KEY_free(ec_key);
        EC_POINT_free(pub_point);
        ECDSA_SIG_free(signature);
        throw std::runtime_error("Failed to DER encode signature.");
    }
    
    // 将vector中的二进制数据转换为std::string
    std::string final_signature(der_signature.begin(), der_signature.end());


    // --- 步骤 4: 清理所有手动分配的OpenSSL对象，防止内存泄漏 ---
    BN_free(bn_private_key);
    EC_KEY_free(ec_key);
    EC_POINT_free(pub_point);
    ECDSA_SIG_free(signature);

    // --- 步骤 5: 返回最终的签名字符串 ---
    return final_signature;
}

std::string serialize_2(std::vector<Input> inputs,std::vector<Output> outputs){
    std::string ss;
    size_t inputs_count =inputs.size();
    ss.write(reinterpret_cast<const char*>(&inputs_count), sizeof(inputs_count));
    for (const auto& input : inputs) {
        size_t txid_len = input.txid.length();
        ss.write(reinterpret_cast<const char*>(&txid_len), sizeof(txid_len));
        ss.write(input.txid.data(), txid_len);

        ss.write(reinterpret_cast<const char*>(&input.index), sizeof(input.index));
    }

    size_t outputs_count = outputs.size();
    ss.write(reinterpret_cast<const char*>(&outputs_count), sizeof(outputs_count));
    for (const auto& output : outputs) {
        ss.write(reinterpret_cast<const char*>(&output.amount), sizeof(output.amount));

        size_t addr_len = output.address.length();
        ss.write(reinterpret_cast<const char*>(&addr_len), sizeof(addr_len));
        ss.write(output.address.data(), addr_len);
    }
    return ss.str();
}

Transaction User::transfer(const std::string &address,const uint64_t &amount)const{
    size_t deposit=0;
    if( address.length()!=64||!hex_verify(address)||amount<=0){
        throw std::invalid_argument("Invalid input");
    }
    //找出交易所需要的UTXO的逻辑
    std::vector<UTXO>my_utxos;
    for(auto &utxo:blockchain_ref.utxo_set){
        if(utxo.second.address==this->wallet_address){
            my_utxos.push_back(utxo.second);//获取UTXO
            deposit+=utxo.second.amount;
            if(deposit>=amount){
                break;
            }
        }
    }
    if(deposit<amount){
        throw std::runtime_error("Insufficient funds.");
    }

    std::vector<Input> inputs;
    std::vector<Output> outputs;
    for(auto &utxo:my_utxos){
        Input input;
        input.index= utxo.index;
        input.txid=utxo.txid;
        input.public_key ;
        input.signature ;
        inputs.push_back(input);
    }
    
    Output output1;
    output1.address =address;
    output1.amount =amount;
    outputs.push_back(output1);
    if(deposit>amount){
        Output output2;
        output2.address =this->wallet_address;
        output2.amount = deposit -amount;
        outputs.push_back(output2);
    }
    std::string hash_to_sign =sha256(serialize_2(inputs, outputs));
    std::string signature =sign(hash_to_sign);
    for(auto &input:inputs){
        input.signature =signature;
        input.public_key=this->public_key;
    }
    return Transaction(inputs,outputs);

};