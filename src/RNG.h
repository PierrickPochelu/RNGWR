#pragma once

#include <stdint.h>
#include <random>
#include <vector>

#include "Strategy.h" // Abstract class

#include "Super_rng.h"



enum StrategyType
{
    XOR,
    BC,
    XH,
    HF1,
    SUPER0,
    SUPER1,
    SUPER2,
    SUPER3,
    SUPER4
};

class RNG
{
public:
    RNG(uint64_t N, uint64_t K);
    RNG(uint64_t N, uint64_t K, StrategyType s);
    RNG(uint64_t N, uint64_t K, StrategyType s, uint64_t seed); // <--- Previlegiate this constructor
    uint64_t it();
    void debug64(uint64_t x);
    uint64_t rand64();
    ~RNG();

    Strategy *getStrategy();
    uint64_t getNumSamples();
    uint64_t getMaxValue();
    uint64_t getMinValue();
    const char *GetName();

private:
    Strategy *Build(StrategyType s);
    Strategy *CreateStrategy(StrategyType s, uint64_t seed);

    uint64_t N; // Values goes from [0,N], thus the number of values up to N+1
    uint64_t K; // We generate K+1 samples
    Strategy *strategy;
};

