#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <random>

// Global seed 
// int inline SEED = 0;
std::random_device rd;
inline int SEED = rd();

#include "constants.hpp"
Constants inline GLOBAL_CONSTANTS; // Global constant set (shared across programs)

#include "operators.hpp"
Operators inline GLOBAL_OPERATORS; 

int inline REGISTER_COUNT = 4;
int inline PROGRAM_LENGTH = 4;
#include "program.hpp"

#include "../evaluate/mse_eval.hpp"

#include "../variate/simple_mutate.hpp"


#endif