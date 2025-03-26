#include <iostream>
#include <cmath>
#include <memory>

#include "../core/default_global.hpp"

double target_function(double x) {
    return std::sin(x);
}

int main() {
    // Register constants
    GLOBAL_CONSTANTS.RegisterConstant(42);
    GLOBAL_CONSTANTS.RegisterConstant(-7);
    GLOBAL_CONSTANTS.RegisterConstant(3.14);
    GLOBAL_CONSTANTS.RegisterConstant(2.71);

    // Register operator 
    GLOBAL_OPERATORS.RegisterUnaryOperator("sin", [](double a){ return std::sin(a); });
    GLOBAL_OPERATORS.RegisterOperator("exp", [](double a, double b) { return std::pow(a,b); });

    // Create test inputs (50 evenly-spaced values from 0 to 2π)
    std::vector<double> inputs;
    for (int i = 0; i < 50; ++i) {
        inputs.push_back(i * (2 * M_PI / 49));
    }

    // Define target function 
    // std::function<double(double)> target_func {static_cast<double(*)(double)>(std::sin)};
    // OR
    std::function<double(double)> target_func {target_function};

    // Declare evaluation method
    std::unique_ptr<Evaluator> evaluator {std::make_unique<MSE>(target_func, inputs, true)};

    // Declare variation method(s)
    // Variation methods are applied in order
    std::vector<std::unique_ptr<Variator>> variators;
    variators.push_back(std::make_unique<SimpleCrossover>(XOVER_RATE));
    variators.push_back(std::make_unique<SimpleMutate>(MUT_RATE));

    // Declare selection method
    std::unique_ptr<Selector> selector {std::make_unique<TournamentSelect>()};

    // Declare program prototype
    std::unique_ptr<Program> prototype {std::make_unique<ArithmeticProgram>()};

    // Declare estimator 
    Estimator est(std::move(evaluator), std::move(variators), std::move(selector), std::move(prototype), true);
    est.Evolve();
    std::cout << est.GetBestProgram();

}