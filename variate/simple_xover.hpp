/* 
Go through each instruction in program
Swap each part of instruction with equal probability
*/

#ifndef SIMPLE_XOVER_HPP
#define SIMPLE_XOVER_HPP

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
    
    std::unique_ptr<Program> Apply(Program const & prog) const override {
        throw std::runtime_error("SimpleCrossover is not a mutation operator.");
    }

    std::unique_ptr<Program> Apply(Program const & p1, Program const & p2) const override {
        // we'll just assume p1 and p2 have the same derived class...
        std::uniform_real_distribution<double> prob_dist(0.0, 1.0);

        std::vector<Instruction> instr1 = p1.GetInstructions();
        std::vector<Instruction> instr2 = p2.GetInstructions();

        if (instr1.size() != instr2.size()) {
            throw std::invalid_argument("Programs must have same number of instructions for crossover.");
        }

        std::vector<Instruction> child_instr;

        for (size_t i {0}; i < instr1.size(); ++i) {
            Instruction const & a {instr1[i]};
            Instruction const & b {instr2[i]};
            
            Instruction temp;
            temp.op = (prob_dist(rng) < xover_rate) ? b.op : a.op;
            temp.Ri = (prob_dist(rng) < xover_rate) ? b.Ri : a.Ri;
            temp.Rj = (prob_dist(rng) < xover_rate) ? b.Rj : a.Rj;

            if (prob_dist(rng) < xover_rate) {
                temp.Rk_type = b.Rk_type;
                temp.Rk = b.Rk;
            }
            else {
                temp.Rk_type = a.Rk_type;
                temp.Rk = a.Rk;
            }
            // temp.Rk = (prob_dist(rng) < xover_rate) ? b.Rk : a.Rk;

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