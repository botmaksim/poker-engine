# Universal Poker Engine (UPIS)

## Project Overview

The Universal Poker Engine (UPIS) is a generalized, high-performance C++17 framework designed for the simulation, evaluation, and algorithmic analysis of structured poker variants. The architecture separates the mechanical state of the game from the heuristic decision engines, establishing a modular environment suitable for large-scale computational game theory experiments and data aggregation. The system currently supports fundamental rulesets, including Texas Hold'em, Omaha, and Short Deck, providing a standardized environment for agent-based modeling and programmatic tournament generation.

## Technical Stack

* **Language:** C++17
* **Build System:** CMake (Version 3.10+)
* **Parallelization:** Standard C++ Runtime Threading (`<thread>`, `<future>`, `<atomic>`)
* **Optimization Methodologies:** Bitwise arithmetic (SIMD Within A Register - SWAR), perfectly hashed logical evaluators, and precomputed heuristic binary trees.

## Compilation and Build Instructions

The engine relies on standard CMake build pipelines. Execute the following commands in the project root to configure and compile the binaries:

```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

The build process generates the primary statically-linked library `libpoker_engine_lib.a` along with the executable toolchains localized in the standard output directories.

## Basic Usage

The environment abstracts structural complexity through the `GameState` and `Variants` namespaces. Standard initialization involves registering an engine instance, configuring the ruleset variant, and initiating the hand simulation protocol:

```cpp
#include "CoreEngine/GameState.hpp"
#include "CoreEngine/Variants/TexasHoldem.hpp"

// Initialize variant ruleset
auto gameVariant = std::make_unique<PokerEngine::Games::TexasHoldem>();
PokerEngine::Engine::GameState state(std::move(gameVariant));

// Register participants with initial chip counts
state.addPlayer(1, 1500);
state.addPlayer(2, 1500);

// Proceed with state logic
state.startHand();
// Interface with AI Inference modules or structural inputs...
```

## Core Analytical Modules

### Monte Carlo Simulator

The evaluation layer utilizes a highly optimized Monte Carlo simulation to estimate positional equity dynamically. Instead of computing combinations iteratively, the pipeline performs threaded hardware-concurrency parallelization over randomized runouts based on known dead cards and opposing constraint distributions. This architectural decision permits convergent sampling of expected win rates for multi-way pots under microsecond constraints.

### Bayesian Inference Tracker

To bridge exploitative counter-strategies with algorithmic baseline play, the engine implements a Bayesian Tracker module. Opposing entities are mapped to a 49-cell discretization matrix characterizing varying degrees of aggression and looseness. Observations of actions (e.g., folding, calling, sizing tendencies) execute constant-time Dirichlet-regularized probability updates across these matrices. The main inference engine blends the Monte Carlo derived equity with the Bayesian state entropy to proportionally evaluate decisions transitioning between pure expected value (EV) and baseline models.
