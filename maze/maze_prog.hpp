#ifndef MAZE_PROG_HPP
#define MAZE_PROG_HPP

#include <memory>
#include <optional>

#include "../core/base_prog.hpp"

class MazeProgram : public Program {
private:
    size_t const min_register_count {6}; // 5 input registers (for sensors) and 1 output register
    size_t register_count, program_length;
    double rk_prob {0.3}; // Hard-coded, probability of including constant over register

    std::vector<double> registers; // Holds register values (ONLY DOUBLE FOR NOW)
    // std::vector<RegisterType> register_types; // Controls register access

    std::vector<Instruction> instructions; // Program instructions

    std::optional<double> fitness;
    
    std::mt19937 rng; // Random number generator

public:
    MazeProgram(size_t rc=6, size_t pl=10)
    : register_count(std::max(rc, min_register_count)),
      program_length(pl),
      rng(SEED)
    {   
        registers = std::vector<double> (register_count, 0.0);
        InitProgram();
    }

    // Necessary for polymorphism
    std::unique_ptr<Program> Clone() const override {
        return std::make_unique<MazeProgram>(*this);
    }

    std::unique_ptr<Program> New() const override {
        std::unique_ptr<Program> p {std::make_unique<MazeProgram>()};
        p->InitProgram();
        return p;
    }

    void InitProgram() override {
        instructions.clear(); // just in case
        instructions = std::vector<Instruction>(program_length);

        std::uniform_real_distribution<double> prob_dist(0.0, 1.0);
        std::uniform_int_distribution<size_t> reg_dist(0, register_count - 1);

        for (Instruction & instr : instructions) {
            instr.Ri = reg_dist(rng);
            instr.Rj = reg_dist(rng);

            instr.op = GLOBAL_OPERATORS.GetRandomOpID();

            // r[k]: Register or Constant?
            if (prob_dist(rng) < rk_prob) {
                // if (prob_dist(rng) < 0.5 && GLOBAL_CONSTANTS.IntSetSize() > 0) { // INT CONSTANT
                //     instr.Rk = {RkType::CONSTANT, GLOBAL_CONSTANTS.GetRandomIntConstant()};
                // }
                // else if (GLOBAL_CONSTANTS.DecSetSize() > 0) { // DEC CONSTANT
                //     instr.Rk = {RkType::CONSTANT, GLOBAL_CONSTANTS.GetRandomDecConstant()};
                // }
                if (GLOBAL_CONSTANTS.Size() > 0) { 
                    instr.Rk_type = RkType::CONSTANT;
                    instr.Rk = GLOBAL_CONSTANTS.GetRandomConstant();
                }
                else { // Fall back to REGISTER INDEX if no constants available
                    instr.Rk_type = RkType::REGISTER;
                    instr.Rk = reg_dist(rng);
                }
            } 
            else { // REGISTER INDEX
                instr.Rk_type = RkType::REGISTER;
                instr.Rk = reg_dist(rng);
            }
        }
    }

    void ExecuteInstruction(Instruction const & instr) {
        // Instructions are represented as r[i] = r[j] op r[k]
        auto op_func = GLOBAL_OPERATORS.GetOperator(instr.op);
        double Rk_value;
        if (instr.Rk_type== RkType::CONSTANT) {
            Rk_value = std::get<double>(instr.Rk);
        }
        else { // if (instr.Rk_type == RkType::REGISTER) {
            Rk_value = registers[std::get<size_t>(instr.Rk)];
        }
        // Registers are clamped to avoid under/overflow
        registers[instr.Ri] = std::clamp(op_func(registers[instr.Rj], Rk_value), -1e6, 1e6);
    }


    double ExecuteProgram() override {
        for (Instruction const & instr : instructions) {
            ExecuteInstruction(instr);
        }
        return std::clamp(registers[0], -1e6, 1e6); // output register
    }


    void Input(std::vector<double> inputs) {
        // Registers 0-4 hold sensor inputs
        // Register 5 hold output
        for (size_t i {0}; i < inputs.size(); ++i) {
            registers[i] = inputs[i];
        }
    }

    void Input(double x) override {
        throw std::runtime_error("Single input is not an option for MazeProgram.");
    }

    double GetOutput() const override {
        return registers[5]; // pure, unaltered
    }


    int GetOutputStep() const { 
        return static_cast<int>(std::round(registers[5])) % 4; // 4 available actions 
    }

    double GetFitness() const override {
        if (!fitness) throw std::runtime_error("Fitness has not been evaluated.");
        return fitness.value(); 
    }
    void SetFitness(double val) override { fitness = val; }
    bool IsEvaluated() const { return fitness.has_value(); }
    void ResetFitness() override { fitness.reset(); }

    void ResetRegisters() override {
        for (double & reg : registers) {
            reg = 0.0;
        }
    }

    std::vector<Instruction> GetInstructions() const override { return instructions; }
    void SetInstructions(std::vector<Instruction> const & in) override { instructions = in; }

    void PrintProgram(std::ostream & os) const override {
        for (Instruction const & instr : instructions) {
            os << "r[" << instr.Ri << "] = r[" << instr.Rj << "] " << 
                GLOBAL_OPERATORS.GetOperatorName(instr.op) << " ";
            if (instr.Rk_type == RkType::REGISTER) {
                os << "r[" << std::get<size_t>(instr.Rk) << "]\n";
            }
            else if (instr.Rk_type == RkType::CONSTANT) {
                os << std::get<double>(instr.Rk) << "\n";
            }
        }
    }
};

#endif