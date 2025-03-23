#ifndef MSE_EVAL_HPP
#define MSE_EVAL_HPP

#include "../core/base_eval.hpp"

#include <cmath>
#include <vector>

class MSE: public Evaluator {
private:
    std::vector<double> test_inputs;
    std::function<double(double)> target_func;

public:
    MSE(std::function<double(double)> func,
        std::vector<double> const & inputs)
        : target_func(func), test_inputs(inputs) { }
    
    double Interpret(Program & prog, double x, bool use_tanh = false) const {
        prog.ResetRegisters();
        prog.Input(x);
        double output {prog.ExecuteProgram()};
        return use_tanh ? std::tanh(output) : output;
    }
    double Evaluate(Program & prog) const override {
        if (test_inputs.empty()) throw std::runtime_error("No test inputs available.");
        double error_sum {0.0};
        for (double x : test_inputs) {
            error_sum += std::pow(Interpret(prog, x) - target_func(x), 2);
        }
        return error_sum / test_inputs.size();
    }
};

#endif