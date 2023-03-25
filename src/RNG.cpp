#include <stdexcept>
#include <math.h>    // C std library. For log2.
#include <algorithm> // Contains "min"

#include <iostream>

#include "Super_rng.h"
#include "RNG.h"

// useful for debugging purpose:
using namespace std;

RNG::RNG(uint64_t N, uint64_t K)
{
    this->N = N;
    this->K = K;

    // Delegated constructor
    cout << "WARNING: No SrategyType given to RNG.";
    cout << "Default strategy set up is 'SUPER1' " << endl;
    strategy = Build(SUPER1);
}

RNG::RNG(uint64_t N, uint64_t K, StrategyType st)
{
    this->N = N;
    this->K = K;
    strategy = Build(st); // Auto seeded with the device
}

RNG::RNG(uint64_t N, uint64_t K, StrategyType st, uint64_t seed)
{
    this->N = N;
    this->K = K;
    strategy = CreateStrategy(st, seed);
}

RNG::~RNG()
{
    delete strategy;
}

Strategy *RNG::Build(StrategyType st)
{
    std::random_device rd;
    uint64_t auto_seed = rd();
    return CreateStrategy(st, auto_seed);
}

Strategy *RNG::CreateStrategy(StrategyType st, uint64_t seed)
{
    Strategy *s = nullptr;
    switch (st)
    {
    case SUPER0:
        s = new Super_rng(N, K, 0, seed);
        break;
    case SUPER1:
        s = new Super_rng(N, K, 1, seed);
        break;
    case SUPER2:
        s = new Super_rng(N, K, 2, seed);
        break;
    case SUPER3:
        s = new Super_rng(N, K, 3, seed);
        break;
    case SUPER4:
        s = new Super_rng(N, K, 4, seed);
        break;
    default:
        std::cerr << "ERROR: Strategy not understood" << std::endl;
        break;
    }
    return s;
}

uint64_t RNG::it()
{
    uint64_t rand_num;
    do
    {
        // strategy produces numbers between [0;2^x[ (if the strategy is base 2) or [0;4^x[ (if the stragy is base 4)
        // For example if N=5 and the strategy is base2, the generator will generate values between [0,8[
        // This is why I use a "loop while" structure for ignoring out-of-range values
        rand_num = strategy->it();

        // std::cout << "i:" << strategy->getI() << " get:" << rand_num << std::endl;
    } while (rand_num > N);

    // std::cout << "return :" << rand_num << std::endl;
    return rand_num;
}

Strategy *RNG::getStrategy()
{
    return strategy;
}

uint64_t RNG::getNumSamples() { return strategy->getNumSamples(); }
uint64_t RNG::getMaxValue() { return strategy->getMaxValue(); }
uint64_t RNG::getMinValue() { return strategy->getMinValue(); }
const char *RNG::GetName() { return strategy->GetName(); }
