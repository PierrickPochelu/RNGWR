#pragma once

#include <stdint.h>
#include <random>

class Strategy // Abstract class pure virtual
{
public:
    Strategy(uint64_t N, uint64_t K, uint64_t seed);
    Strategy(uint64_t N, uint64_t K);
    virtual uint64_t it() = 0; // Pure Virtual
    virtual const char *GetName() const = 0;
    void debug64(uint64_t x); // From number to its binary representation
    uint64_t rand64();

    uint64_t getNumSamples();
    uint64_t getMaxValue();
    uint64_t getMinValue();
    uint64_t getI() const;
    // Warning: Values goes from 0 inclusively and getModulus() exclusively if N<2**64-1, otherwise getModulus() is inclusive.
protected:
    // Mersenne Twister 64-bit version to generate a random number with a high degree of randomness.
    std::mt19937_64 rng;
    std::uniform_int_distribution<uint64_t> distr;

    uint64_t N;
    uint64_t K;

    uint64_t modulus;
    uint64_t nb_samples;

    uint64_t num_bits;
    uint64_t i;

    const uint64_t MAX_UINT64 = 0xFFFFFFFFFFFFFFFFull;

private:
    void init_deterministic();
    void init_rng(uint64_t seed);
    void init_rng();
    void init_i();
};

