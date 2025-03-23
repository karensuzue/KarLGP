#include <iostream>
#include <cmath>
#include "core/global.hpp"

int main() {

    // Register some constants
    GLOBAL_CONSTANTS.RegisterConstant(42);
    GLOBAL_CONSTANTS.RegisterConstant(-7);
    GLOBAL_CONSTANTS.RegisterConstant(3.14);
    GLOBAL_CONSTANTS.RegisterConstant(2.71);
    // std::cout << GLOBAL_CONSTANTS;

    // std::cout << "Size of global constants set: " << GLOBAL_CONSTANTS.Size() << std::endl;
    // std::cout << "Random constant: " << GLOBAL_CONSTANTS.GetRandomConstant() << std::endl;

    // std::cout << "Seed: " << SEED << std::endl;


    // All operators must take two parameters for syntax consistency.
    // For unary operators, we ignore the second parameter.
    // GLOBAL_OPERATORS.RegisterUnaryOperator("sin", [](double a){ return sin(a); });
    // size_t id2 = GLOBAL_OPERATORS.GetRandomOpID();
    // auto func2 = GLOBAL_OPERATORS.GetOperator(id2);
    // std::string name2 = GLOBAL_OPERATORS.GetOperatorName(id2);

    // std::cout << "Random global operator: " << name2 << std::endl;
    // std::cout << "Result of 1 and 4: " << func2(1, 4) << std::endl;
    // std::cout << GLOBAL_OPERATORS;

    Program program;
    std::cout << program;
    
    program.Input(5);
    double output = program.ExecuteProgram();
    std::cout << output << std::endl;


    std::vector<double> xs;
    for (int i = 0; i < 50; ++i) {
        xs.push_back(i * (2 * M_PI / 49)); // from 0 to 2π
    }
    MSE evaluator(static_cast<double(*)(double)>(std::sin), xs);
    program.SetFitness(evaluator.Evaluate(program));
    std::cout << "Program Fitness: " << program.GetFitness() << std::endl;

    SimpleMutate mutator(0.1);
    mutator.Apply(program);
    std::cout << program;

    return 0;
}