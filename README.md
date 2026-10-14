# Universal Poker Engine: High-Performance UPIS Game Theory Framework

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![CMake 3.16+](https://img.shields.io/badge/CMake-3.16%2B-blue.svg)](https://cmake.org/)
[![Architecture](https://img.shields.io/badge/Architecture-Zero%20Allocation-orange.svg)](#)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

The Universal Poker Engine is a generalized, zero-allocation C++17 framework designed for poker variant simulations, hand evaluations, and game-theoretic tree solving. The system adheres to the Universal Poker Interface Specification, enabling plug-and-play integration with external research solvers and bot harnesses.

---

## Table of Contents
1. [Framework Capabilities](#framework-capabilities)
2. [Engine Architecture](#engine-architecture)
3. [Prerequisites](#prerequisites)
4. [Build & Benchmarking](#build--benchmarking)
5. [Directory Structure](#directory-structure)
6. [License](#license)

---

## Framework Capabilities

* **High-Throughput Hand Evaluator**: Bitboard card representation and lookup tables evaluating millions of hand combinations per second.
* **Variant Generalization**: Configurable state machine supporting Texas Hold'em, Omaha, and custom multi-street betting structures.
* **Zero-Allocation Core**: Pre-allocated memory arenas eliminating heap churn during high-depth Monte Carlo simulations.
* **UPIS Protocol Bridge**: Standardized interface communication conforming to tournament poker bot protocols.

---

## Engine Architecture

* **`include/`**: Public headers defining card bitmasks, evaluator lookups, and state machine transitions.
* **`src/`**: Concrete implementations of game trees, equity calculators, and protocol parsers.
* **`tools/`**: Automated verification benchmarks and Monte Carlo accuracy validation scripts.

---

## Prerequisites

* Compiler: GCC 9+ or Clang 10+ with C++17 support
* Build System: CMake 3.16+

---

## Build & Benchmarking

```bash
git clone git@github.com:botmaksim/poker-engine.git
cd poker-engine

cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

./build/poker_engine_bench
```

---

## Directory Structure

```text
poker-engine/
├── CMakeLists.txt           # Build automation configuration
├── include/                 # UPIS framework public headers
├── src/                     # Core engine and hand evaluation implementations
└── tools/                   # Benchmarks and validation harnesses
```

---

## License

This project is open-source software licensed under the [MIT License](LICENSE).
