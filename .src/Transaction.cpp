#include"Transaction.h"
Transaction::Transaction(std::vector<Input> inputs,std::vector<Output> outputs):inputs(inputs),outputs(outputs){
    this->txid = convert_2_16(sha256(this->serialize()));
};


std::string Transaction::serialize() const {
    std::stringstream ss;
    size_t inputs_count = this->inputs.size();
    ss.write(reinterpret_cast<const char*>(&inputs_count), sizeof(inputs_count));
    for (const auto& input : this->inputs) {
        size_t txid_len = input.txid.length();
        ss.write(reinterpret_cast<const char*>(&txid_len), sizeof(txid_len));
        ss.write(input.txid.data(), txid_len);

        ss.write(reinterpret_cast<const char*>(&input.index), sizeof(input.index));

        size_t sig_len = input.signature.length();
        ss.write(reinterpret_cast<const char*>(&sig_len), sizeof(sig_len));
        ss.write(input.signature.data(), sig_len);

        size_t pubkey_len = input.public_key.length();
        ss.write(reinterpret_cast<const char*>(&pubkey_len), sizeof(pubkey_len));
        ss.write(input.public_key.data(), pubkey_len);
    }

    size_t outputs_count = this->outputs.size();
    ss.write(reinterpret_cast<const char*>(&outputs_count), sizeof(outputs_count));
    for (const auto& output : this->outputs) {
        ss.write(reinterpret_cast<const char*>(&output.amount), sizeof(output.amount));

        size_t addr_len = output.address.length();
        ss.write(reinterpret_cast<const char*>(&addr_len), sizeof(addr_len));
        ss.write(output.address.data(), addr_len);
    }
    return ss.str();
}