#include <stdexcept>
#include <math.h> // C std library. For log2.

// useful for debugging purpose:
#include <bitset>
using namespace std;
#include <iostream>

#include "Super_rng.h"
#include "RNG.h"

Super_rng::Super_rng(uint64_t N, uint64_t K, uint64_t l, uint64_t seed) : Strategy(N, K, seed)
{
    level = l;
    init();
}

Super_rng::Super_rng(uint64_t N, uint64_t K, uint64_t l) : Strategy(N, K)
{
    level = l;
    init();
}

Super_rng::~Super_rng() {}

void Super_rng::init()
{

    // Force the number of bits to be even (base 4)
    num_bits_base_4 = num_bits;
    if (num_bits_base_4 % 2 == 1)
    {
        num_bits_base_4 += 1;
    }
    half_bits_base_4 = num_bits_base_4 / 2;

    if (num_bits < 64)
    {
        xor_key = Strategy::rand64() % (1ull << num_bits_base_4);
    }
    else
    {
        xor_key = rand64();
    }

    /*  **** Bit Concat Init ***** */

    i = 0; // We don't need a random offset for this method
    if (num_bits < 64)
    {
        limit_N_binary = 1 << (num_bits);
    }
    else
    {
        limit_N_binary = MAX_UINT64;
    }

    const uint64_t num_bits_i_for_k = (uint64_t)std::max((uint64_t)std::ceil(std::log2(nb_samples)), (uint64_t)1);

    // Example A: if N=5 -> modulus=6 num_bits==3. K=4 -> nb_samples=5 we need 3 bits.
    // We may need to ignore 2 value max: 7, 6 . num_ignored_values=2
    // num_bits_for_K_and_ignored_values=log2(num_ignored_values+nb_samples+1)=3

    // Example B: if N=100 -> modulus=101, num_bits=7. K=50 -> nb_samples=51 we need 6bits.
    //  We may ignored 27 values 101, 102, 103, ... 127.
    //  num_bits_for_K_and_ignored_values=log2(num_ignored_values+nb_samples+1)=log2(27+51+1)=7
    //
    uint64_t num_bits_for_K_and_ignored_values, num_ignored_values;
    if (num_bits_base_4 < 64)
    {
        num_ignored_values = (1ull << num_bits_base_4) - modulus;
    }
    else
    {
        num_ignored_values = MAX_UINT64 - modulus;
    }

    num_bits_for_K_and_ignored_values = (uint64_t)std::ceil(std::log2(num_ignored_values + nb_samples + 1));

    if (nb_samples < MAX_UINT64 && num_bits_for_K_and_ignored_values < MAX_UINT64)
    {
        limit_K_binary = (1ull << num_bits_for_K_and_ignored_values);
        control_mask = limit_K_binary - 1;
    }
    else
    {
        limit_K_binary = MAX_UINT64;
        control_mask = limit_K_binary;
    }

    /*  **** Feistel recursive Init ***** */

    build_keys_recurs(half_bits_base_4, 1, recursive_keys, min_recusive_word_size);

    for (uint64_t I = 0; I < fc_rounds; I++)
    {
        uint64_t random = Strategy::rand64() % (1ull << half_bits_base_4);
        fc_keys.push_back((uint64_t)random);
    }
}

const char *Super_rng::GetName() const
{
    static char name[8]; // warning, we assume we will never use "SuperX" with X >= 10
    snprintf(name, sizeof(name), "Super%lu", level);
    return name;
}

uint64_t Super_rng::symmetry(uint64_t x) const
{
    for (uint64_t I = 0; I < half_bits_base_4; I++)
    { // remember: "i" is the iteration variable name
        uint64_t J = num_bits - I - 1;
        uint64_t bit_i = (x >> I) & 1;
        uint64_t bit_j = (x >> J) & 1;
        uint64_t tmp = bit_i ^ bit_j;
        tmp = (tmp << I) | (tmp << J);
        x = tmp ^ x;
    }
    return x;
}

uint64_t Super_rng::hadamard(uint64_t x) const
{
    uint64_t L = x >> half_bits_base_4; // when x="11110" L="011" R="110"
    uint64_t R = x & ((1ull << half_bits_base_4) - 1ull);

    for (uint64_t r = 0; r < had_rounds; r++)
    {

        // Apply 1 round
        uint64_t Rnext = (L + 2ull * R) % (1ull << half_bits_base_4);
        uint64_t Lnext = (L + R) % (1ull << half_bits_base_4);

        // Update for next round
        R = Rnext;
        L = Lnext;
    }

    // Concatenate R and L
    uint64_t y = (R << half_bits_base_4) | L;
    return y;
}

uint64_t Super_rng::bitconcat(uint64_t x)
{ // limit_N_binary, control_mask, random_part are base 2 (the number of bits is any positive integer)
    uint64_t random_part;
    if (limit_N_binary < MAX_UINT64)
    {
        random_part = Strategy::rand64() % limit_N_binary;
    }
    else
    {
        random_part = Strategy::rand64();
    }

    uint64_t out = (~control_mask & random_part) | ((control_mask)&i);

    return out;
}

uint64_t Super_rng::feistel(uint64_t x) const
{
    uint64_t L = x >> half_bits_base_4;
    uint64_t R = x & ((1ULL << half_bits_base_4) - 1);

    // For each round of Feistel Cipher
    for (uint64_t r = 0; r < fc_rounds; r++)
    {
        uint64_t key = fc_keys[r];
        uint64_t Rnext = L ^ (R ^ key);
        uint64_t Lnext = R;
        R = Rnext;
        L = Lnext;
    }

    uint64_t y = (R << half_bits_base_4) | L;
    return y;
}

uint64_t feister_f(
    uint64_t x,
    uint64_t id,
    uint64_t half_bits_base_4,
    unordered_map<uint64_t, uint64_t> &fc_keys,
    uint64_t MIN_WORD_SIZE);
uint64_t feister_f(
    uint64_t x,
    uint64_t id,
    uint64_t half_bits_base_4,
    unordered_map<uint64_t, uint64_t> &fc_keys,
    uint64_t MIN_WORD_SIZE)
{
    uint64_t L = x >> half_bits_base_4;
    uint64_t R = x & ((1ULL << half_bits_base_4) - 1);

    // For each round of Feistel Cipher

    // should be half_bits_base_4
    // cout << "I need:  " << id << endl;
    uint64_t key = fc_keys.find((uint64_t)id)->second;
    // cout << "i got it" << endl;

    // one single round
    uint64_t Rnext = L ^ (R ^ key);
    uint64_t Lnext = R;
    R = Rnext;
    L = Lnext;

    if (half_bits_base_4 <= MIN_WORD_SIZE)
    {
        return (R << half_bits_base_4) | L;
    }
    else
    {
        L = feister_f(L, 2 * id, half_bits_base_4 / 2, fc_keys, MIN_WORD_SIZE);
        R = feister_f(R, 2 * id + 1, half_bits_base_4 / 2, fc_keys, MIN_WORD_SIZE);
        return (R << half_bits_base_4) | L;
    }
}

void Super_rng::build_keys_recurs(uint64_t num_bits,
                                  uint64_t id, // the root is id=1  . Each child is 2*id and 2*id + 1. -> allows to identify nodes with an integer
                                  unordered_map<uint64_t, uint64_t> &keys,
                                  const uint64_t MIN_WORD_SIZE)
{
    // add one number
    uint64_t random_part;
    if (num_bits < 64)
    {
        random_part = Strategy::rand64() % (1ull << num_bits);
    }
    else
    {
        random_part = Strategy::rand64();
    }
    // cout << "insert: " << id << endl;
    keys.insert({id, random_part});

    if (num_bits < MIN_WORD_SIZE)
    {
        // nothing
    }
    else
    {
        // recursion
        build_keys_recurs(num_bits / 2, 2 * id, keys, MIN_WORD_SIZE);
        build_keys_recurs(num_bits / 2, 2 * id + 1, keys, MIN_WORD_SIZE);
    }
}

uint64_t Super_rng::it()
{
    uint64_t out;

    out = bitconcat(i);

    if (level == 0)
    {
        // out = out ^ xor_key;
        // out = symmetry(out);
    }
    else if (level == 1)
    {
        out = symmetry(out);
        out = hadamard(out);
        out = feistel(out);
        out = symmetry(out);
    }
    else if (level == 2)
    {
        out = symmetry(out); // uniform

        out = hadamard(out);                                                               // shuffle
        out = feister_f(out, 1, half_bits_base_4, recursive_keys, min_recusive_word_size); // suffle but create local patterns
        out = symmetry(out);                                                               // erase local patterns
    }
    else if (level == 3)
    {
        out = symmetry(out);
        for (int I = 0; I < 4; I++)
        {
            out = hadamard(out);                                                               // shuffle
            out = feister_f(out, 1, half_bits_base_4, recursive_keys, min_recusive_word_size); // suffle but create local patterns
            out = symmetry(out);                                                               // erase local patterns
        }
    }
    else if (level == 4)
    {
        out = symmetry(out);
        for (int I = 0; I < 128; I++)
        {
            out = hadamard(out);                                                               // shuffle
            out = feister_f(out, 1, half_bits_base_4, recursive_keys, min_recusive_word_size); // suffle but create local patterns
            out = symmetry(out);                                                               // erase local patterns
        }
    }

    i++;
    return out;
}
