#include "Transform.h"
//将16进制字符转化为10进制整数
int hexCharToInt(char c){
    char upper_c = std::toupper(c);
    if(upper_c<='9'&&upper_c>='0'){
        return upper_c -'0';
    }
    else if(upper_c>='A'&&upper_c<='F'){
        return upper_c -'A'+10;
    }
    else {
        // 如果是无效的十六进制字符
        throw std::invalid_argument("Invalid hexadecimal character: " + std::string(1, c));
    }
}
//将16进制字符串转化为32字节的二进制字符串
std::string convert_16_2(const std::string&hash){
    std::string result;
    result.reserve(32);
    for (size_t i = 0; i <64; i+=2)
    {
        int value =hexCharToInt(hash[i])*16+hexCharToInt(hash[i+1]);
        result.push_back(value);
    }
    return result;
}
 //对得到的二进制数据块转化为16进制的哈希值字符串
std::string convert_2_16(const std::string&data){
    std::stringstream result;
    result<<std::hex<<std::setfill('0');
    for(unsigned char c:data){
        result<<std::setw(2)<<static_cast<int>(c);
    }
    return result.str();
}