#include "User.h"
#include "Hash.h"
#include"Transaction.h"
#include <vector>

// For big number arithmetic (BIGNUM)
std::string User::sign(const std::string& hash_to_sign) const {
    // --- Setup custom deleters for OpenSSL objects ---
    auto bn_deleter = [](BIGNUM* p) { BN_free(p); };
    auto ec_key_deleter = [](EC_KEY* p) { EC_KEY_free(p); };
    auto ec_point_deleter = [](EC_POINT* p) { EC_POINT_free(p); };
    auto ecdsa_sig_deleter = [](ECDSA_SIG* p) { ECDSA_SIG_free(p); };

    // --- Step 1: Load our private key into OpenSSL's EC_KEY structure (RAII version) ---
    std::unique_ptr<BIGNUM, decltype(bn_deleter)> bn_private_key(
        BN_bin2bn(reinterpret_cast<const unsigned char*>(this->private_key.c_str()), this->private_key.length(), nullptr),
        bn_deleter
    );
    if (!bn_private_key) {
        throw std::runtime_error("Failed to convert private key to BIGNUM.");
    }

    std::unique_ptr<EC_KEY, decltype(ec_key_deleter)> ec_key(
        EC_KEY_new_by_curve_name(NID_secp256k1),
        ec_key_deleter
    );
    if (!ec_key) {
        throw std::runtime_error("Failed to create new EC_KEY.");
    }

    if (EC_KEY_set_private_key(ec_key.get(), bn_private_key.get()) != 1) {
        throw std::runtime_error("Failed to set private key.");
    }

    const EC_GROUP* group = EC_KEY_get0_group(ec_key.get());
    std::unique_ptr<EC_POINT, decltype(ec_point_deleter)> pub_point(EC_POINT_new(group), ec_point_deleter);
    if (!pub_point) {
        throw std::runtime_error("Failed to create public key point.");
    }

    if (EC_POINT_mul(group, pub_point.get(), bn_private_key.get(), nullptr, nullptr, nullptr) != 1) {
        throw std::runtime_error("Failed to calculate public key point.");
    }

    if (EC_KEY_set_public_key(ec_key.get(), pub_point.get()) != 1) {
        throw std::runtime_error("Failed to set public key.");
    }

    // --- Step 2: Perform signing ---
    std::unique_ptr<ECDSA_SIG, decltype(ecdsa_sig_deleter)> signature(
        ECDSA_do_sign(reinterpret_cast<const unsigned char*>(hash_to_sign.c_str()), hash_to_sign.length(), ec_key.get()),
        ecdsa_sig_deleter
    );
    if (!signature) {
        throw std::runtime_error("Failed to generate ECDSA signature.");
    }

    // --- Step 3: Serialize the signature into a DER-encoded string ---
    int der_sig_len = i2d_ECDSA_SIG(signature.get(), nullptr);
    if (der_sig_len <= 0) {
        throw std::runtime_error("Failed to determine DER signature length.");
    }

    std::vector<unsigned char> der_signature(der_sig_len);
    unsigned char* p = der_signature.data();

    if (i2d_ECDSA_SIG(signature.get(), &p) <= 0) {
        throw std::runtime_error("Failed to DER encode signature.");
    }

    return std::string(der_signature.begin(), der_signature.end());
}

std::string serialize_2(const std::vector<Input> &inputs,const std::vector<Output> &outputs){
    std::stringstream ss;
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
    if( address.length()!=64||!hex_verify(address)||amount<=0){
        throw std::invalid_argument("Invalid input");
    }

    // Logic to find the UTXOs needed for the transaction
    uint64_t deposit=0;
    std::vector<UTXO> my_utxos;
    for(const auto &utxo_pair:blockchain_ref.utxo_set){
        if(utxo_pair.second.address==this->wallet_address){
            my_utxos.push_back(utxo_pair.second); // Get UTXO
            deposit+=utxo_pair.second.amount;
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

    // Create outputs
    outputs.push_back(Output{amount, address});
    if(deposit>amount){
        outputs.push_back(Output{deposit - amount, this->wallet_address});
    }

    // Create a temporary list of inputs without signatures to generate the hash
    for(const auto &utxo:my_utxos){
        inputs.push_back(Input{utxo.txid, utxo.index, "", ""});
    }

    // Sign the transaction hash
    std::string hash_to_sign = sha256(serialize_2(inputs, outputs));
    std::string signature = sign(hash_to_sign);

    // Add signature and public key to all inputs
    for(auto &input:inputs){
        input.signature = signature;
        input.public_key = this->public_key;
    }
    
    return Transaction(inputs,outputs);
}