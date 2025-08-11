
#include "Hash.h"
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <openssl/ec.h>      // 椭圆曲线加密的核心库
#include <openssl/obj_mac.h> // 包含了曲线的ID (NID_secp256k1)
#include <openssl/rand.h>  // 用于生成安全的随机数
#include <openssl/ecdsa.h>   
#include <openssl/bn.h>      // 用于大数运算
#include <memory>            // 用于智能指针 std::unique_ptr
#include <vector>
#include <string>
#include "Transform.h"
//用来将SHA256返回的无符号数据转化为字符串
std::string sha256(const std::string& data) {
    unsigned char hash_buffer[SHA256_DIGEST_LENGTH];
    SHA256(
        reinterpret_cast<const unsigned char*>(data.c_str()),
        data.length(),
        hash_buffer
    );
    return std::string(
        reinterpret_cast<const char*>(hash_buffer),
        SHA256_DIGEST_LENGTH
    );
}


// --- 这是核心的密钥生成函数 ---
KeyPair generate_keys() {
    auto deleter_ec_key = [](EC_KEY* p) { EC_KEY_free(p); };
    std::unique_ptr<EC_KEY, decltype(deleter_ec_key)> ec_key(EC_KEY_new_by_curve_name(NID_secp256k1), deleter_ec_key);
    if (!ec_key) {
        throw std::runtime_error("Failed to create new EC_KEY.");
    }
    // --- 1. 生成密钥对 ---
    if (EC_KEY_generate_key(ec_key.get()) != 1) {
        throw std::runtime_error("Failed to generate EC key pair.");
    }

    // --- 2. 提取私钥 ---
    const BIGNUM* private_bn = EC_KEY_get0_private_key(ec_key.get());
    if (!private_bn) {
        throw std::runtime_error("Failed to get private key BIGNUM.");
    }
    // 将BIGNUM格式的私钥转换为32字节的二进制字符串
    std::vector<unsigned char> private_key_vec(BN_num_bytes(private_bn));
    BN_bn2bin(private_bn, private_key_vec.data());
    // 确保私钥是32字节，不足则在前面补0
    if (private_key_vec.size() < 32) {
        private_key_vec.insert(private_key_vec.begin(), 32 - private_key_vec.size(), 0);
    }
    std::string private_key_bin(private_key_vec.begin(), private_key_vec.end());


    // --- 3. 提取公钥 ---
    const EC_POINT* public_point = EC_KEY_get0_public_key(ec_key.get());
    if (!public_point) {
        throw std::runtime_error("Failed to get public key point.");
    }
    // 将EC_POINT格式的公钥转换为未压缩的二进制字符串 (通常是65字节，以0x04开头)
    auto deleter_bn_ctx = [](BN_CTX* p) { BN_CTX_free(p); };
    std::unique_ptr<BN_CTX, decltype(deleter_bn_ctx)> ctx(BN_CTX_new(), deleter_bn_ctx);
    if (!ctx) {
        throw std::runtime_error("Failed to create BN_CTX.");
    }
    size_t pubkey_len = EC_POINT_point2oct(EC_KEY_get0_group(ec_key.get()), public_point, POINT_CONVERSION_UNCOMPRESSED, nullptr, 0, ctx.get());
    std::vector<unsigned char> public_key_vec(pubkey_len);
    EC_POINT_point2oct(EC_KEY_get0_group(ec_key.get()), public_point, POINT_CONVERSION_UNCOMPRESSED, public_key_vec.data(), pubkey_len, ctx.get());
    std::string public_key_bin(public_key_vec.begin(), public_key_vec.end());

    // --- 4. 生成钱包地址 ---
    // 简化版地址生成：直接对公钥进行哈希，然后转为十六进制
    // (真实比特币地址会更复杂：SHA256 -> RIPEMD160 -> Base58Check编码)
    std::string address_hash_bin = sha256(public_key_bin);
    std::string wallet_address_hex = convert_2_16(address_hash_bin);
    return {private_key_bin, public_key_bin, wallet_address_hex};
}


std::string public_key_to_address(const std::string& public_key_bin) {
    std::string address_hash_bin = sha256(public_key_bin);
    std::string wallet_address_hex = convert_2_16(address_hash_bin);
    return wallet_address_hex;
}

bool verify_signature(const std::string& public_key_bin, 
                      const std::string& signature_der, 
                      const std::string& hash_to_sign) {

    // --- 步骤 1: 将二进制的公钥加载到OpenSSL的EC_KEY结构中 ---
    
    // 1a. 创建一个空的EC_KEY对象，并指定曲线
    auto deleter_ec_key = [](EC_KEY* p) { EC_KEY_free(p); };
    std::unique_ptr<EC_KEY, decltype(deleter_ec_key)> ec_key(EC_KEY_new_by_curve_name(NID_secp256k1), deleter_ec_key);
    if (!ec_key) {
        // 无法创建密钥对象，直接返回失败
        return false;
    }

    // 1b. 将二进制公钥字符串转换为OpenSSL内部的EC_POINT格式
    const unsigned char* pub_key_ptr = reinterpret_cast<const unsigned char*>(public_key_bin.c_str());
    EC_KEY* raw_key = ec_key.get();
    if (o2i_ECPublicKey(&raw_key, &pub_key_ptr, public_key_bin.length()) == NULL) {
        // 转换失败，说明公钥格式不正确
        return false;
    }

    // --- 步骤 2: 执行验证 ---
    
    // ECDSA_do_verify 是OpenSSL的核心验证函数
    // 它会用 ec_key 中的公钥，来验证 signature_der 是否是对 hash_to_sign 的有效签名
    // 返回值: 1 = 验证成功, 0 = 验证失败, -1 = 出现错误
    int result = ECDSA_verify(
        0, // type, 默认为0
        reinterpret_cast<const unsigned char*>(hash_to_sign.c_str()),
        hash_to_sign.length(),
        reinterpret_cast<const unsigned char*>(signature_der.c_str()),
        signature_der.length(),
        ec_key.get()
    );

    // --- 步骤 3: 返回最终结果 ---
    // 只有当返回值明确为1时，才代表签名有效
    return result == 1;
}
