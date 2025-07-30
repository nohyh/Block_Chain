cmake_minimum_required(VERSION 3.15)

project(BLOCKCHAIN_SYSTEM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)

add_executable(
    BLOCKCHAIN_SYSTEM
    .src/Block.cpp 
    .src/BlockChain.cpp
    .src/Transaction.cpp
    .src/Miner.cpp
    .src/User.cpp
    .src/Transform.cpp
    .src/Hash.cpp
    .src/main.cpp
)
target_include_directories(BLOCKCHAIN_SYSTEM PRIVATE .src)

target_link_libraries(
    BLOCKCHAIN_SYSTEM
    OpenSSL::SSL
    OpenSSL::Crypto
)