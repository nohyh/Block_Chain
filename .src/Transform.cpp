#include "Transform.h"
#include <string>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <memory>
#include <openssl/bn.h>
#include <sstream>
#include <iomanip>

// Convert a hexadecimal character to a decimal integer
int hexCharToInt(char c){
    char upper_c = std::toupper(c);
    if(upper_c<='9'&&upper_c>='0'){
        return upper_c -'0';
    }
    else if(upper_c>='A'&&upper_c<='F'){
        return upper_c -'A'+10;
    }
    else {
        // If it is an invalid hexadecimal character
        throw std::invalid_argument("Invalid hexadecimal character: " + std::string(1, c));
    }
}
// Convert a hexadecimal string to a 32-byte binary string
std::string convert_16_2(const std::string&hash){
    std::string result;
    result.reserve(32);
    for (size_t i = 0; i <64; i+=2)
    {
        int value =hexCharToInt(hash[i])*16+hexCharToInt(hash[i+1]);
        result.push_back(static_cast<char>(value));
    }
    return result;
}
 // Convert the obtained binary data block into a hexadecimal hash string
std::string convert_2_16(const std::string&data){
    std::stringstream result;
    result<<std::hex<<std::setfill('0');
    for(unsigned char c:data){
        result<<std::setw(2)<<static_cast<int>(c);
    }
    return result.str();
}

// Check if the string consists entirely of hexadecimal numbers
bool hex_verify(const std::string &address){
    for(char ch:address){
        if((ch>='0'&&ch<='9')||(ch>='a'&&ch<='f')||(ch>='A'&&ch<='F')){
            continue;
        }
        else{
            return false;
        }
    }
    return true;
}

const char* BASE58_ALPHABET = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

std::string base58_encode(const std::string& data) {
    // 1. Convert string to BIGNUM
    auto bn_deleter = [](BIGNUM* p) { BN_free(p); };
    std::unique_ptr<BIGNUM, decltype(bn_deleter)> bn(BN_new(), bn_deleter);
    BN_bin2bn(reinterpret_cast<const unsigned char*>(data.c_str()), data.size(), bn.get());

    // 2. Setup for division
    auto ctx_deleter = [](BN_CTX* p) { BN_CTX_free(p); };
    std::unique_ptr<BN_CTX, decltype(ctx_deleter)> ctx(BN_CTX_new(), ctx_deleter);
    std::unique_ptr<BIGNUM, decltype(bn_deleter)> dv(BN_new(), bn_deleter);
    std::unique_ptr<BIGNUM, decltype(bn_deleter)> rem(BN_new(), bn_deleter);
    BN_set_word(dv.get(), 58);

    std::string result = "";

    // 3. Repeatedly divide by 58
    while (!BN_is_zero(bn.get())) {
        BN_div(bn.get(), rem.get(), bn.get(), dv.get(), ctx.get());
        result += BASE58_ALPHABET[BN_get_word(rem.get())];
    }

    std::reverse(result.begin(), result.end());

    // 4. Add leading '1's for leading zero bytes
    int nLeadingZeros = 0;
    for (size_t i = 0; i < data.size() && data[i] == 0; ++i) {
        nLeadingZeros++;
    }
    return std::string(nLeadingZeros, '1') + result;
}

std::string format_amount(uint64_t amount) {
    double formatted_amount = static_cast<double>(amount) / 1000000000.0;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(8) << formatted_amount;
    return ss.str();
}