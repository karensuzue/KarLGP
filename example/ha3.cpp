#include <iostream>
#include <cmath>
#include <memory>

#include "../core/default_global.hpp"

// Approximate sin(x)
double target_function(double x) {
    return std::sin(x);
}

int main() {
    GLOBAL_CONSTANTS.RegisterConstant(-1);
    GLOBAL_CONSTANTS.RegisterConstant(1);
    GLOBAL_CONSTANTS.RegisterConstant(3.1416);
    GLOBAL_CONSTANTS.RegisterConstant(6.2832);
    GLOBAL_CONSTANTS.RegisterConstant(2);
    GLOBAL_CONSTANTS.RegisterConstant(0.5);
    GLOBAL_CONSTANTS.RegisterConstant(-0.5);
    GLOBAL_CONSTANTS.RegisterConstant(0);

    // Create training inputs (50 evenly-spaced values from 0 to 2π)
    std::vector<double> inputs;
    for (int i {0}; i < 50; ++i) {
        inputs.push_back(i * (2 * M_PI / 49));
    }

    std::unique_ptr<Evaluator> evaluator {std::make_unique<MSE>(target_function, inputs, true)};

    // Comment out the operators you don't need
    std::vector<std::unique_ptr<Variator>> variators;
    variators.push_back(std::make_unique<SimpleCrossover>(XOVER_RATE));
    variators.push_back(std::make_unique<SimpleMutate>(MUT_RATE));
    // variators.push_back(std::make_unique<RandomVariator>());

    std::unique_ptr<Selector> selector {std::make_unique<TournamentSelect>()};
    
    std::unique_ptr<Program> prototype {std::make_unique<ArithmeticProgram>()};

    Estimator est(std::move(evaluator), std::move(variators), std::move(selector), std::move(prototype), false);

    est.MultiRunEvolve();
    // ArithmeticProgram test;
    // std::cout << test;
    // std::cout << test.StructuralIntronProp() << std::endl;
    // std::cout << test.SemanticIntronProp(*evaluator) << std::endl;
    // std::cout << test.SemanticIntronProp_Elimination(*evaluator) << std::endl;
}


