# RNGWR


RNGWR (Random Number Generator Without Repetition) is a C++ program that generates a sequence of random numbers without repetition. The program offers four different strategies for generating random numbers, named 'SUPER1', 'SUPER2', 'SUPER3', and 'SUPER4', which balance between computing speed and random quality using a mix of sampling and cryptographic techniques. They all are O(1) in computing and O(1) in memory and therefore usefull for generating efficiently millions or billions of numbers.

## Simple utilization

To use the program, you can check the main file located at ./bin/main.cpp. To compile the program, simply run the 'make' command in the terminal. Once compiled, you can run the program using the command './bin/program'.

The program generates a sequence of random numbers, and each sequence is unique and does not repeat any numbers. For example, when num_samples=5 and max_values=5 the program may output "1 5 2 3 0", "0 4 3 2 5" or "1 4 0 5 2", depending on the random seed.

## Benchmark

The command 'make test' generate the program './bin/test_program'. It will run unit tests, and produces OPERM5 test based on chi2, uniform test based on chi2 and the computing speed (micro-seconds) for generating 10,000 numbers.

