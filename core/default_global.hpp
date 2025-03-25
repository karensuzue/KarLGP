#ifndef DEF_GLOBAL_HPP
#define DEF_GLOBAL_HPP

#include <random>

// ----PARAMETERS (DON'T MODIFY NAMES)----

// Global seed 
// int inline SEED = 0;
std::random_device rd;
int inline SEED = rd();
size_t inline REGISTER_COUNT = 10;
size_t inline PROGRAM_LENGTH = 10;
size_t inline POP_SIZE = 100;
size_t inline GENS = 10;
size_t inline TOUR_SIZE = 4;


// ----DON'T TOUCH----

#include "constants.hpp"
Constants inline GLOBAL_CONSTANTS; // Global constant set (shared across programs)

#include "operators.hpp"
Operators inline GLOBAL_OPERATORS; 

// #include "estimator.hpp"


// ----REGISTER MODULES----

#include "arith_prog.hpp"

#include "../evaluate/mse_eval.hpp"

#include "../variate/simple_mutate.hpp"
#include "../variate/simple_xover.hpp"

#include "../select/tour_select.hpp"


#endif