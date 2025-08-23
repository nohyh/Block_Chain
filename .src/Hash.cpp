
#include "Hash.h"
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <sstream>
#include <iomanip>
#include <openssl/ec.h>      // Core library for elliptic curve cryptography
#include <openssl/obj_mac.h> // Contains the curve ID (NID_secp256k1)
#include <openssl/rand.h>  // For generating secure random numbers
#include <openssl/ecdsa.h>   
#include <openssl/bn.h>      // For big number arithmetic
#include <memory>            // For smart pointers std::unique_ptr
#include <vector>
#include <string>
#include "Transform.h"
// Used to convert the unsigned char data returned by SHA256 into a string
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

std::string ripemd160(const std::string& data) {
    unsigned char hash_buffer[RIPEMD160_DIGEST_LENGTH];
    RIPEMD160(
        reinterpret_cast<const unsigned char*>(data.c_str()),
        data.length(),
        hash_buffer
    );
    return std::string(
        reinterpret_cast<const char*>(hash_buffer),
        RIPEMD160_DIGEST_LENGTH
    );
}


// --- This is the core key generation function ---
KeyPair generate_keys() {
    auto deleter_ec_key = [](EC_KEY* p) { EC_KEY_free(p); };
    std::unique_ptr<EC_KEY, decltype(deleter_ec_key)> ec_key(EC_KEY_new_by_curve_name(NID_secp256k1), deleter_ec_key);
    if (!ec_key) {
        throw std::runtime_error("Failed to create new EC_KEY.");
    }
    // --- 1. Generate key pair ---
    if (EC_KEY_generate_key(ec_key.get()) != 1) {
        throw std::runtime_error("Failed to generate EC key pair.");
    }

    // --- 2. Extract private key ---
    const BIGNUM* private_bn = EC_KEY_get0_private_key(ec_key.get());
    if (!private_bn) {
        throw std::runtime_error("Failed to get private key BIGNUM.");
    }
    // Convert the BIGNUM format private key to a 32-byte binary string
    std::vector<unsigned char> private_key_vec(32, 0);
    BN_bn2binpad(private_bn, private_key_vec.data(), private_key_vec.size());
    std::string private_key_bin(private_key_vec.begin(), private_key_vec.end());


    // --- 3. Extract public key ---
    const EC_POINT* public_point = EC_KEY_get0_public_key(ec_key.get());
    if (!public_point) {
        throw std::runtime_error("Failed to get public key point.");
    }
    // Convert the EC_POINT format public key to an uncompressed binary string (usually 65 bytes, starting with 0x04)
    auto deleter_bn_ctx = [](BN_CTX* p) { BN_CTX_free(p); };
    std::unique_ptr<BN_CTX, decltype(deleter_bn_ctx)> ctx(BN_CTX_new(), deleter_bn_ctx);
    if (!ctx) {
        throw std::runtime_error("Failed to create BN_CTX.");
    }
    size_t pubkey_len = EC_POINT_point2oct(EC_KEY_get0_group(ec_key.get()), public_point, POINT_CONVERSION_UNCOMPRESSED, nullptr, 0, ctx.get());
    std::vector<unsigned char> public_key_vec(pubkey_len);
    EC_POINT_point2oct(EC_KEY_get0_group(ec_key.get()), public_point, POINT_CONVERSION_UNCOMPRESSED, public_key_vec.data(), pubkey_len, ctx.get());
    std::string public_key_bin(public_key_vec.begin(), public_key_vec.end());

    // --- 4. Generate wallet address ---
    std::string wallet_address = public_key_to_address(public_key_bin);
    return KeyPair{private_key_bin, public_key_bin, wallet_address};
}


std::string public_key_to_address(const std::string& public_key_bin) {
    // Step 1 & 2: SHA-256 then RIPEMD-160
    std::string hash1 = sha256(public_key_bin);
    std::string hash2 = ripemd160(hash1);

    // Step 3: Add version byte (0x00 for mainnet)
    char version = 0x00;
    std::string hash3 = version + hash2;

    // Step 4 & 5: Double SHA-256 for checksum
    std::string hash4 = sha256(hash3);
    std::string hash5 = sha256(hash4);
    std::string checksum = hash5.substr(0, 4);

    // Step 6: Append checksum
    std::string hash6 = hash3 + checksum;

    // Step 7: Base58Check encode
    return base58_encode(hash6);
}

bool verify_signature(const std::string& public_key_bin, 
                      const std::string& signature_der, 
                      const std::string& hash_to_sign) {

    // --- Step 1: Load the binary public key into OpenSSL's EC_KEY structure ---
    
    // 1a. Create an empty EC_KEY object and specify the curve
    auto deleter_ec_key = [](EC_KEY* p) { EC_KEY_free(p); };
    std::unique_ptr<EC_KEY, decltype(deleter_ec_key)> ec_key(EC_KEY_new_by_curve_name(NID_secp256k1), deleter_ec_key);
    if (!ec_key) {
        // Failed to create key object, return false directly
        return false;
    }

    // 1b. Convert the binary public key string to OpenSSL's internal EC_POINT format
    const unsigned char* pub_key_ptr = reinterpret_cast<const unsigned char*>(public_key_bin.c_str());
    EC_KEY* raw_key = ec_key.get();
    if (o2i_ECPublicKey(&raw_key, &pub_key_ptr, public_key_bin.length()) == NULL) {
        // Conversion failed, indicating the public key format is incorrect
        return false;
    }

    // --- Step 2: Perform verification ---
    
    // ECDSA_do_verify is the core verification function of OpenSSL
    // It uses the public key in ec_key to verify if signature_der is a valid signature for hash_to_sign
    // Return value: 1 = verification successful, 0 = verification failed, -1 = error occurred
    int result = ECDSA_verify(
        0, // type, defaults to 0
        reinterpret_cast<const unsigned char*>(hash_to_sign.c_str()),
        hash_to_sign.length(),
        reinterpret_cast<const unsigned char*>(signature_der.c_str()),
        signature_der.length(),
        ec_key.get()
    );

    // --- Step 3: Return the final result ---
    // Only a return value of 1 represents a valid signature
    return result == 1;
}
