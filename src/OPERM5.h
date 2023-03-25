#pragma once

#include <cstring> //contains std::memcopy
#include <cstdint>
#include <vector>

float OPERM5Test(uint64_t *rnd, uint64_t n);
double uniform(std::vector<uint64_t> samples, uint64_t min_val, uint64_t max_val);