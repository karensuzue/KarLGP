/* 
Go through each instruction in program
Swap each part of instruction with equal probability
*/

#ifndef SIMPLE_XOVER_HPP
#define SIMPLE_XOVER_HPP

#include <cassert>
#include <random>
#include <vector>
#include <memory>

#include "../core/base_vari.hpp"

class SimpleCrossover: public Variator {
private:
    double xover_rate;
    std::mt19937 mutable rng;
public:
    SimpleCrossover(double rate) : xover_rate(rate), rng(SEED) { }

    VariatorType Type() const override { return VariatorType::BINARY; }
    
    std::unique_ptr<Program> Apply(Program const & ) const override {
        assert(false && "SimpleCrossover is not a unary operator.");
        return std::unique_ptr<Program>();
    }

    std::unique_ptr<Program> Apply(Program const & p1, Program const & p2) const override {
        // we'll just assume p1 and p2 have the same derived class...
        std::uniform_real_distribution<double> prob_dist(0.0, 1.0);

        std::vector<Instruction> instr1 = p1.GetInstructions();
        std::vector<Instruction> instr2 = p2.GetInstructions();

        assert (instr1.size() == instr2.size() &&
         "Programs must have same number of instructions for crossover.");

        std::vector<Instruction> child_instr;

        for (size_t i {0}; i < instr1.size(); ++i) {
            Instruction const & a {instr1[i]};
            Instruction const & b {instr2[i]};
            
            Instruction temp;
            // Choose operator
            if (prob_dist(rng) < xover_rate) {
                temp.op = b.op;
                temp.op_type = b.op_type;
            }
            else {
                temp.op = a.op;
                temp.op_type = a.op_type;
            }

            // Choose destination register
            temp.Ri = (prob_dist(rng) < xover_rate) ? b.Ri : a.Ri;

            // Choose operand (j - register only)
            temp.Rj = (prob_dist(rng) < xover_rate) ? b.Rj : a.Rj;

            // Choose operand (t - register only, only for ternary operators)
            if (a.op_type == 1 && b.op_type == 1 && temp.op_type == 1) {
                // If both instructions are ternary
                temp.Rt = (prob_dist(rng) < xover_rate) ? b.Rt : a.Rt;
            }
            else if (a.op_type == 1 && b.op_type == 0 && temp.op_type == 1) {
                // If only instruction a is ternary
                temp.Rt = a.Rt;
            }
            else if (b.op_type == 1 && a.op_type == 0 && temp.op_type == 1) {
                // If only instruction b is ternary
                temp.Rt = b.Rt;
            }

            // Choose operand (k - register OR constant)
            if (prob_dist(rng) < xover_rate) {
                temp.Rk_type = b.Rk_type;
                temp.Rk = b.Rk;
            }
            else {
                temp.Rk_type = a.Rk_type;
                temp.Rk = a.Rk;
            }

            child_instr.push_back(temp);
        }

        std::unique_ptr<Program> child {p1.Clone()};
        child->SetInstructions(child_instr);
        child->ResetFitness();
        child->ResetRegisters();
        return child;
    }
};

#endif