/* 
Go through each instruction in program
Mutate each part of instruction with equal probability
*/

#ifndef SIMPLE_MUTATE_HPP
#define SIMPLE_MUTATE_HPP

#include <random>
#include <vector>

#include "../core/base_vari.hpp"

class SimpleMutate: public Variator {
private:
    double mutation_rate;
    std::mt19937 mutable rng;
public:
    SimpleMutate(double rate) : mutation_rate(rate), rng(SEED) { }
    
    void Apply(Program & prog) const override {
        std::uniform_real_distribution<double> prob_dist(0.0, 1.0);
        std::uniform_int_distribution<size_t> reg_dist(0, REGISTER_COUNT - 1);

        std::vector<Instruction> instructions = prog.GetInstructions();

        for (Instruction & instr : instructions) {
            if (prob_dist(rng) < mutation_rate) instr.op = GLOBAL_OPERATORS.GetRandomOpID();
            if (prob_dist(rng) < mutation_rate) instr.Ri = reg_dist(rng);
            if (prob_dist(rng) < mutation_rate) instr.Rj = reg_dist(rng);
            if (prob_dist(rng) < mutation_rate) {
                if (instr.Rk.first == RkType::CONSTANT) {
                    if (prob_dist(rng) < 0.5 && GLOBAL_CONSTANTS.Size() > 0) {
                        instr.Rk.second = GLOBAL_CONSTANTS.GetRandomConstant();
                    }
                    else {
                        instr.Rk = {RkType::REGISTER, reg_dist(rng)};
                    }
                }
                else {
                    if (prob_dist(rng) < 0.5) {
                        instr.Rk.second = reg_dist(rng);
                    }
                    else if (GLOBAL_CONSTANTS.Size() > 0) {
                        instr.Rk = {RkType::CONSTANT, GLOBAL_CONSTANTS.GetRandomConstant()};
                    }

                }
            }
        }

        prog.SetInstructions(instructions);

    }
    Program Apply(Program const & p1, Program const & p2) const override {
        throw std::runtime_error("SimpleMutate is not a crossover operator.");
    }
};

#endif