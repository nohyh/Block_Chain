# MiniChain: A Blockchain Implementation from Scratch in C++

<p align="center">
  <img alt="Language" src="https://img.shields.io/github/languages/top/nohyh/Block_Chain?style=for-the-badge&color=blue">
  <img alt="Code size" src="https://img.shields.io/github/repo-size/nohyh/Block_Chain?style=for-the-badge&color=green">
  <img alt="License" src="https://img.shields.io/github/license/nohyh/Block_Chain?style=for-the-badge&color=lightgrey">
</p>

> A minimal blockchain simulation built from scratch using C++20 to demonstrate and learn the core principles of a Bitcoin-like system, including transactions, mining, and consensus.

---

### 🎉 Project Status: Completed (v1.0)

This project is now feature-complete and has been successfully tested. It serves as a functional demonstration of core blockchain concepts and was developed as an educational exercise. While it is complete, it is intended for learning purposes and may not be production-ready.

---

## ✨ Core Features

* **Wallet & Transactions**: Key pair generation and transaction signing using OpenSSL (ECDSA).
* **UTXO Model**: A full implementation of the Unspent Transaction Output (UTXO) model for balance management.
* **Concurrent Mining (PoW)**: Simulates competitive mining using C++ multi-threading and a Proof-of-Work algorithm.
* **Blocks & Chain**: Implements a block structure with a Merkle Root, which is then verified and linked into a chain.
* **Network Simulation**: A transaction pool temporarily stores transactions waiting to be mined by competing miners.

## 🛠️ Tech Stack

* **Language**: C++20
* **Build System**: CMake
* **Dependencies**: OpenSSL
* **Concurrency**: C++ Standard Library (`<thread>`, `<mutex>`, `<atomic>`)

## 🚀 Getting Started

### 1. Prerequisites

Ensure you have a C++20 compatible compiler, CMake (version 3.15 or higher), and OpenSSL development libraries installed.

**On Ubuntu/Debian:**
```bash
sudo apt update && sudo apt install build-essential cmake libssl-dev
```
**On macOS (using Homebrew):**
```bash
brew install cmake openssl
```

### 2. Build and Run

```bash
# Clone the repository
git clone https://github.com/nohyh/Block_Chain.git
cd Block_Chain

# Create build directory and compile
mkdir build && cd build
cmake ..
make

# Run the simulation
./BLOCKCHAIN_SYSTEM
```
Once running, the console will display real-time information about auto-generated transactions and newly mined blocks.

## 📜 License

This project is licensed under the [MIT](LICENSE) License.