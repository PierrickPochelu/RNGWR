#pragma once

#include <stdint.h>
#include "Strategy.h"
#include <unordered_map>

using namespace std;


class Super_rng : public Strategy
{
public:
    Super_rng(uint64_t N, uint64_t K, uint64_t level, uint64_t seed);
    Super_rng(uint64_t N, uint64_t K, uint64_t level);
    void init();
    uint64_t it();
    const char* GetName() const;
    ~Super_rng();
void build_keys_recurs(uint64_t num_bits, 
uint64_t id,
unordered_map<uint64_t, uint64_t>& keys,
const uint64_t MIN_WORD_SIZE
);
private:
    uint64_t num_bits_base_4;
    uint64_t half_bits_base_4;
    uint64_t half_bits;

    // XOR Cipher
    uint64_t xor_key;

    // Feister settings
    const uint64_t fc_rounds=1;
    vector<uint64_t> fc_keys;
    uint64_t level=0;
    std::unordered_map<uint64_t , uint64_t > recursive_keys;
    const uint64_t min_recusive_word_size=2;

    // From BitConcat
    uint64_t limit_N_binary;
    uint64_t control_mask;
    uint64_t limit_K_binary;

    uint64_t bitconcat(uint64_t x);
    uint64_t feistel(uint64_t x) const;
    uint64_t symmetry(uint64_t x) const;
    uint64_t hadamard(uint64_t x) const;
    const uint64_t had_rounds=1; // Does not systematically improves the OPERM5 metrics, but increases the uniform distrib.
};
