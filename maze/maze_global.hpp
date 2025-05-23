#ifndef MAZE_GLOBAL_HPP
#define MAZE_GLOBAL_HPP

#include <random>

#include <random>

// ----PARAMETERS (DON'T MODIFY NAMES)----

// Global seed 
// constexpr int SEED = 0;
std::random_device rd;
int SEED = rd();
constexpr size_t REGISTER_COUNT = 10;
constexpr size_t PROGRAM_LENGTH = 10;
constexpr size_t ELITISM_COUNT = 10;
constexpr size_t POP_SIZE = 200;
constexpr size_t GENS = 1000;
constexpr size_t TOUR_SIZE = 5;
constexpr double XOVER_RATE = 0.1;
constexpr double MUT_RATE = 0.1;


// ----MUST INCLUDE----

#include "../core/constants.hpp"
Constants GLOBAL_CONSTANTS; // Global constant set (shared across programs)

#include "../core/operators.hpp"
Operators GLOBAL_OPERATORS(true); // ternary operators are enabled


// ----REGISTER MODULES----

#include "maze_prog.hpp"
#include "maze_env.hpp"

#include "maze_eval.hpp"
#include "maze_novelty_eval.hpp"
// #include "maze_surprise_eval.hpp"

#include "../variate/simple_mutate.hpp"
#include "../variate/simple_xover.hpp"

#include "../select/tour_select.hpp"



// ----MUST INCLUDE----

#include "../core/estimator.hpp"

#endif