#ifndef MAZE_GLOBAL_HPP
#define MAZE_GLOBAL_HPP

#include <random>

#include <random>

// ----PARAMETERS (DON'T MODIFY NAMES)----

// Global seed 
// constexpr int SEED = 0;
std::random_device rd;
int SEED = rd();
constexpr size_t REGISTER_COUNT = 4;
constexpr size_t PROGRAM_LENGTH = 10;
constexpr size_t POP_SIZE = 1000;
constexpr size_t GENS = 500;
constexpr size_t TOUR_SIZE = 40;
constexpr double XOVER_RATE = 0.1;
constexpr double MUT_RATE = 0.1;


// ----MUST INCLUDE----

#include "constants.hpp"
Constants GLOBAL_CONSTANTS; // Global constant set (shared across programs)

#include "operators.hpp"
Operators GLOBAL_OPERATORS; 


// ----REGISTER MODULES----

#include "arith_prog.hpp"

#include "../evaluate/mse_eval.hpp"

#include "../variate/simple_mutate.hpp"
#include "../variate/simple_xover.hpp"

#include "../select/tour_select.hpp"


// ----MUST INCLUDE----

#include "estimator.hpp"

#endif