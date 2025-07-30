
#include "Hash.h"
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
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
