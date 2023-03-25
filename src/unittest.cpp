#include <algorithm>
#include <iostream>
#include <string>
#include <math.h>
#include <assert.h>
#include <random>
#include <chrono>

#include <cmath> // contains gamma function in C++17
#include <set>

// For timing below code
#include <chrono>
#include <thread>

#include "RNG.h"
#include "OPERM5.h"

uint64_t test_speed(const uint64_t N, const uint64_t K, const size_t runs, StrategyType st)
{
    long cumul_t = 0;

    // just for extracting name
    RNG generator{N, K, st, 0};
    const char *name = generator.GetName();

    for (uint64_t r = 0; r < runs; ++r)
    {
        RNG generator{N, K, st, r};
        uint64_t n;
        // Timer t1 = GetTimer();
        auto t1 = std::chrono::system_clock::now();

        for (uint64_t i = 0; i < K; ++i)
        {
            n = generator.it();
        }
        auto t2 = std::chrono::system_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();

        cumul_t += elapsed_time;
    }

    uint64_t mean_ms = (cumul_t / runs) / 1000;
    printf("TIME TEST: %s N: %lu K: %lu T(us): %lu\n", name, N, K, mean_ms);
    return mean_ms;
}

void visual_inspection(const uint64_t N, const uint64_t K, const size_t runs, StrategyType st)
{
    for (uint64_t r = 0; r < runs; ++r)
    {
        RNG generator{N, K, st, r};
        printf("%s with N: %lu K: %lu \n", generator.GetName(), N, K);
        uint64_t n;
        for (uint64_t i = 0; i < K; ++i)
        {
            n = generator.it();
            printf("%lu\n", n);
        }
    }
}

uint64_t test_random_seed_effect(const uint64_t N, const uint64_t K, const size_t runs, StrategyType st)
{
    // WARNING: put N large, K small, runs>1.
    RNG generator(N, K, st, 0);
    const char *name = generator.GetName();
    uint64_t num_samples = generator.getNumSamples();
    uint64_t fails = 0;

    // TEST 1 : FIXED SEED
    uint64_t fixed_random_seed = 1;
    std::set<uint64_t> first_number;
    std::set<uint64_t> last_number;
    for (uint64_t r = 0; r < runs; ++r)
    {
        RNG generator(N, K, st, fixed_random_seed);

        std::set<uint64_t> unique_numbers;
        for (int i = 0; i < generator.getNumSamples(); i++)
        {
            uint64_t rand_num = generator.it();

            if (i == 0)
            {
                first_number.insert(rand_num);
            }
            else if (i == num_samples - 1)
            {
                last_number.insert(rand_num);
            }
        }
    }
    if (first_number.size() > 1 or last_number.size() > 1)
    {
        printf("FIXED RANDOM SEED FAIL: %s N: %lu K: %lu \n", name, N, K);
        fails += 1;
    }

    first_number.clear();
    last_number.clear();
    for (uint64_t r = 0; r < runs; ++r)
    {
        RNG generator(N, K, st, r);
        for (int i = 0; i < generator.getNumSamples(); i++)
        {
            uint64_t rand_num = generator.it();

            if (i == 0)
            {
                first_number.insert(rand_num);
            }
            else if (i == num_samples - 1)
            {
                last_number.insert(rand_num);
            }
        }
    }
    if (first_number.size() == 1 or last_number.size() == 1)
    {
        printf("VARYING RANDOM SEED FAIL: %s N: %lu K: %lu \n", name, N, K);
        fails += 1;
    }

    return fails;
}

float test_operm5(const uint64_t N, const uint64_t K, const size_t runs, StrategyType st)
{
    float chi2_cumul = 0;
    char *name;

    for (uint64_t r = 0; r < runs; ++r)
    {
        RNG generator{N, K, st, r};

        // Store results to test
        std::vector<uint64_t> results_vector;
        results_vector.reserve(generator.getNumSamples());

        for (size_t i = 0; i < generator.getNumSamples(); ++i)
        {
            results_vector.emplace_back(generator.it());
        }

        float chi2 = OPERM5Test(results_vector.data(), results_vector.size());
        chi2_cumul += chi2;
        name = (char *)generator.GetName();
    }

    // OPERM5 check
    float score = chi2_cumul / runs; // mean chi2 on runs test
    printf("%s K=%lu N=%lu OPERM5=%.2f\n", name, K, N, score);
    return score;
}

float test_uniform(const uint64_t N, const uint64_t K, const size_t runs, StrategyType st)
{
    float stop_rate = 0.125; // We will evaluate if it is uniform only on 12.5% first values
    double chi2_cumul = 0;
    char *name;

    for (uint64_t r = 0; r < runs; ++r)
    {
        RNG generator{N, K, st, r};

        // Store results to test
        std::vector<uint64_t> results_vector;
        results_vector.reserve(generator.getNumSamples());

        for (uint64_t i = 0; i < (uint64_t)(stop_rate * generator.getNumSamples()); ++i)
        {
            results_vector.emplace_back(generator.it());
        }

        double chi2 = uniform(results_vector, generator.getMinValue(), generator.getMaxValue());
        chi2_cumul += chi2;
        name = (char *)generator.GetName();
    }

    double score = chi2_cumul / runs; // mean chi2 on runs test
    printf("%s K=%lu N=%lu Uniform=%.4f\n", name, K, N, score);
    return score;
}

uint64_t test_no_repeat(uint64_t N, uint64_t K, uint64_t runs, StrategyType st)
{
    char *name;
    uint64_t n;
    uint64_t fails = 0;
    for (uint64_t i = 0; i < runs; ++i)
    {
        RNG generator(N, K, st, i);
        n = 0;
        // Generate K numbers
        std::set<uint64_t> unique_numbers;
        for (int i = 0; i < generator.getNumSamples(); i++)
        {
            uint64_t rand_num = generator.it();
            unique_numbers.insert(rand_num);
            n++;
        }

        // Check that the set contains K unique numbers
        if (unique_numbers.size() != generator.getNumSamples() or unique_numbers.size() != n)
        {
            printf("REPET. FAIL: %s N: %lu K: %lu \n", generator.GetName(), N, K);
            fails += 1;
        }
    }
    return fails;
}

uint64_t test_N_K_API(uint64_t N, uint64_t K, uint64_t runs, StrategyType st)
{
    char *name;
    uint64_t n;
    uint64_t fails = 0;
    for (uint64_t i = 0; i < runs; ++i)
    {
        RNG generator(N, K, st, i);

        // Test that RNG produces number inferior or equal to N
        for (int i = 0; i < generator.getNumSamples(); i++)
        {
            uint64_t rand_num = generator.it();
            if (rand_num > N)
            {
                printf("N FAILS: rand_num > N \n");
                fails += 1;
            }
        }

        // Check that the RNG is able to produce K+1 or K numbers
        if (generator.getNumSamples() != K + 1 || (generator.getNumSamples() == K && K == UINT64_MAX))
        {
            printf("K FAILS: generator.getNumSamples() != K+1 : %lu != %lu \n", generator.getNumSamples(), K + 1);
            fails += 1;
        }
    }
    return fails;
}

void SHORT_UNIT_TEST(std::vector<StrategyType> &strategies)
{
    uint64_t runs = 3;
    for (const StrategyType &strat : strategies)
    {
        test_no_repeat(0, 0, runs, strat);
        test_no_repeat(1, 1, runs, strat);
        test_no_repeat(1, 0, runs, strat);
        test_no_repeat(3, 3, runs, strat);
        test_no_repeat(4, 4, runs, strat);
        test_no_repeat(5, 4, runs, strat);

        test_no_repeat(10, 2, runs * 5, strat);
        test_no_repeat(100, 10, runs * 10, strat);
        test_no_repeat(100, 50, runs * 2, strat);
        test_no_repeat(100, 100, runs, strat);

        test_no_repeat(127, 127, runs, strat);
        test_no_repeat(128, 128, runs, strat);
        test_no_repeat(255, 255, runs, strat);
        test_no_repeat(256, 256, runs, strat);
    }

    for (const StrategyType &strat : strategies)
    {

        test_N_K_API(0, 0, runs, strat);
        test_N_K_API(1, 1, runs, strat);
        test_N_K_API(3, 3, runs, strat);
        test_N_K_API(5, 4, runs, strat);

        test_N_K_API(10, 2, runs * 5, strat);
        test_N_K_API(100, 10, runs * 10, strat);
        test_N_K_API(100, 50, runs, strat);
        test_N_K_API(100, 100, runs, strat);

        test_N_K_API(127, 127, runs, strat);
        test_N_K_API(128, 128, runs, strat);
        test_N_K_API(255, 255, runs, strat);
        test_N_K_API(256, 256, runs, strat);
    }
}

void BIG_UNIT_TEST(std::vector<StrategyType> &strategies)
{
    uint64_t runs = 1;

    for (const StrategyType &strat : strategies)
    {
        test_no_repeat(100 * 1000000, 1000000, runs, strat);
        test_no_repeat(0xFFFFFFFFFFFFFFFFull, 100000, runs, strat);
        test_no_repeat(1000000, 10000, runs, strat);
        test_no_repeat(1000, 1000, runs, strat);
    }

    for (const StrategyType &strat : strategies)
    {
        test_N_K_API(100 * 1000000, 1000000, runs, strat);
        test_N_K_API(0xFFFFFFFFFFFFFFFFull, 100000, runs, strat);
        test_N_K_API(1000000, 10000, runs, strat);
        test_N_K_API(1000, 1000, runs, strat);
    }
}

void RANDOM_TEST(std::vector<StrategyType> &strategies)
{
    // Test random seed behaviour:
    // * Fixed random seed -> same series
    // * Diff random seed -> chance to get different series
    for (const StrategyType &strat : strategies)
    {
        test_random_seed_effect(0xFFFFFFFFFFFFFFFFull, 3, 10, strat);
    }

    // OPERM5 test for each method, varying N and K
    uint64_t b8 = 255;
    uint64_t b16 = 65535;
    uint64_t b32 = 4294967295;
    uint64_t b64 = 0xFFFFFFFFFFFFFFFFull;
    uint64_t runs = 10;

    for (const StrategyType &strat : strategies)
    {
        test_operm5(b8, b8, runs, strat);

        test_operm5(b16, b8, runs, strat);
        test_operm5(b16, b16, runs, strat);

        test_operm5(100 * 1000000, 1000000, 1, strat);

        test_operm5(b32, b8, runs, strat);
        test_operm5(b32, b16, runs, strat);

        test_operm5(0xFFFFFFFFFFFFFFFFull, b8, runs, strat);
        test_operm5(0xFFFFFFFFFFFFFFFFull, b16, runs, strat);
    }

    for (const StrategyType &strat : strategies)
    {
        test_uniform(b8, b8, runs, strat);

        test_uniform(b16, b8, runs, strat);
        test_uniform(b16, b16, runs, strat);

        test_uniform(100 * 1000000, 1000000, 1, strat);

        test_uniform(b32, b8, runs, strat);
        test_uniform(b32, b16, runs, strat);

        test_uniform(0xFFFFFFFFFFFFFFFFull, b8, runs, strat);
        test_uniform(0xFFFFFFFFFFFFFFFFull, b16, runs, strat);
    }
}

int main(int argc, char *argv[])
{
    std::vector<StrategyType> strategies = {SUPER1, SUPER2, SUPER3, SUPER4};

    printf("Short unit tests ... \n");
    SHORT_UNIT_TEST(strategies);
    printf("Big unit tests (takes several minutes)... \n");
    BIG_UNIT_TEST(strategies);
    printf("Random test score (takes several minutes) ... \n");
    RANDOM_TEST(strategies);

    // Time test
    uint64_t b064 = 0xFFFFFFFFFFFFFFFFull;
    printf("Benchmark time. May take a few seconds ... \n");
    for (const StrategyType &strat : strategies)
    {
        test_speed(b064, 10000, 1, strat);
    }

    /*
     // Visual inspection
     for(const StrategyType& strat : strategies){
         visual_inspection(63, 63, 1, strat);
         // you can copy past in an tabular software to display curve (e.g. Calc, Excel...)
     }
     */

    return EXIT_SUCCESS;
}
