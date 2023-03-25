#include <type_traits>
#include <cstdint>
#include <random>
#include <iostream>
#include <bitset>

#include "Strategy.h"

using namespace std;

Strategy::Strategy(uint64_t N, uint64_t K, uint64_t seed) : N(N), K(K)
{
    init_deterministic();
    init_rng(seed);
    init_i();
}

Strategy::Strategy(uint64_t N, uint64_t K) : N(N), K(K)
{
    init_deterministic();
    init_rng();
    init_i();
}

void Strategy::init_deterministic()
{
    if (N == MAX_UINT64)
    {
        modulus = static_cast<uint64_t>(N); // avoid arithmetic error
    }
    else
    {
        modulus = static_cast<uint64_t>(N) + 1;
    }
    if (K == MAX_UINT64)
    {
        nb_samples = static_cast<uint64_t>(K); // avoid arithmetic error
    }
    else
    {
        nb_samples = static_cast<uint64_t>(K) + 1;
    }

    if (N == 0 or N == 1)
        num_bits = 1;
    else
        num_bits = (uint64_t)std::ceil(std::log2(modulus));
}

void Strategy::init_rng(uint64_t seed)
{
    rng = std::mt19937_64(seed);                       // initialize the random number engine with the given seed
    distr = std::uniform_int_distribution<uint64_t>(); // define the distribution for uint64_t values
}

void Strategy::init_rng()
{
    std::random_device rd;                             // obtain a random seed from the OS
    rng = std::mt19937_64(rd());                       // initialize the random number engine with the seed
    distr = std::uniform_int_distribution<uint64_t>(); // define the distribution for uint64_t values
}

void Strategy::init_i()
{
    if ((modulus - nb_samples) > 0)
    {
        i = rand64() % (modulus - nb_samples); // Random initial position
    }
    else
    {
        i = 0;
    }
}

// Generally usefull functions
uint64_t Strategy::rand64()
{
    return distr(rng);
}

void Strategy::debug64(uint64_t x)
{
    std::bitset<std::numeric_limits<uint64_t>::digits> bitx(x);
    std::cout << bitx << " : " << x << std::endl;
}

uint64_t Strategy::getNumSamples() { return nb_samples; }
uint64_t Strategy::getMaxValue()
{
    if (modulus < MAX_UINT64)
        return modulus - 1;
    else
        return MAX_UINT64;
}
uint64_t Strategy::getMinValue()
{
    return 0ull;
}
uint64_t Strategy::getI() const { return i; }
