#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <cctype>
#include<string>
#include<sstream>
#include <iomanip>

int hexCharToInt(char c);

std::string convert_16_2(const std::string&hash);

std::string convert_2_16(const std::string&data);

#endif