
#include <algorithm>
#include <iostream>
#include <string>
#include <math.h>
#include <assert.h>
#include <random>
#include <chrono>

#include "RNG.h"

int main(int argc, char* argv[])
{
    const uint64_t max_values=5;
    const uint64_t nb_samples=5;

    RNG rng(max_values, nb_samples, SUPER1);
    
    for (int i=0;i<nb_samples;i++){
        uint64_t random_num = rng.it();
        printf("%lu \n", random_num);
    }
}
