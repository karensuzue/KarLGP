#ifndef MSE_EVAL_HPP
#define MSE_EVAL_HPP

#include "../core/base_eval.hpp"

#include <cmath>
#include <vector>

class MSE: public Evaluator {
private:
    std::vector<double> test_inputs;
    std::function<double(double)> target_func;

    bool use_tanh;

public:
    MSE(std::function<double(double)> func,
        std::vector<double> const & inputs,
        bool tanh=false)
        : target_func(func), test_inputs(inputs), use_tanh(tanh) { }
    
    double Interpret(Program & prog, double x) const {
        prog.ResetRegisters();
        prog.Input(x);
        double output {prog.ExecuteProgram()};
        output = use_tanh ? std::tanh(output) : output;
        // NaN still passes through std::clamp as NaN
        // Penalty for overflow (similar to PyshGP)
        return std::isfinite(output) ? std::clamp(output, -1e6, 1e6) : 1e6;
    }
    double Evaluate(Program & prog) const override {
        if (test_inputs.empty()) throw std::runtime_error("No test inputs available.");
        double error_sum {0.0};
        for (double x : test_inputs) {
            double pred {Interpret(prog, x)};
            double targ {target_func(x)};

            // if (std::isnan(pred) || std::isinf(pred)) {
            //     std::cerr << "NaN detected! Input = " << x << "\n";
            //     prog.PrintProgram(std::cerr);
            //     return 1e6;
            // }
            // double diff {pred - targ};
            
            error_sum += std::pow(pred-targ, 2);
        }
        return error_sum / test_inputs.size();
    }
};

#endif