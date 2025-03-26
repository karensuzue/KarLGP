#ifndef ARITH_PROG_HPP
#define ARITH_PROG_HPP

#include <vector>
#include <variant>
#include <random>
#include <optional>
#include <algorithm>

#include "base_prog.hpp"

class ArithmeticProgram : public Program {
private:
    size_t register_count;
    size_t program_length;
    double rk_prob {0.3}; // Hard-coded

    std::vector<double> registers; // Holds register values (ONLY DOUBLE FOR NOW)
    std::vector<RegisterType> register_types; // Controls register access

    std::vector<Instruction> instructions; // Program instructions

    std::optional<double> fitness;
    
    std::mt19937 rng; // Random number generator

public:
    ArithmeticProgram() : register_count(REGISTER_COUNT), program_length(PROGRAM_LENGTH), rng(SEED) { 
        registers = std::vector<double>(register_count, 0);
        register_types = std::vector<RegisterType>(register_count, RegisterType::NORMAL);

        // r[1] holds the input
        // r[0] holds the return value
        register_types[1] = RegisterType::READ_ONLY;

        InitProgram();
    }

    ArithmeticProgram(size_t rg_count, size_t prog_len) 
    : register_count(rg_count), program_length(prog_len), rng(SEED) {
        registers = std::vector<double>(register_count, 0);
        register_types = std::vector<RegisterType>(register_count, RegisterType::NORMAL);

        // r[1] holds the input
        // r[0] holds the return value
        register_types[1] = RegisterType::READ_ONLY;

        InitProgram();
    }

    std::unique_ptr<Program> Clone() const override {
        return std::make_unique<ArithmeticProgram>(*this);
    }

    std::unique_ptr<Program> New() const override {
        std::unique_ptr<Program> p {std::make_unique<ArithmeticProgram>()};
        p->InitProgram(); // just in case
        return p;
    }

    bool IsEvaluated() const { return fitness.has_value(); }
    double GetFitness() const override { 
        if (!fitness) throw std::runtime_error("Fitness has not been evaluated.");
        return fitness.value(); 
    }
    void SetFitness(double val) override { fitness = val; }
    void ResetFitness() override { fitness.reset(); }

    // std::vector<Instruction> & GetInstructions() { return instructions; } 
    std::vector<Instruction> GetInstructions() const override { return instructions; }
    void SetInstructions(std::vector<Instruction> const & in) override { instructions = in; }

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
                    instr.Rk = {RkType::CONSTANT, GLOBAL_CONSTANTS.GetRandomConstant()};
                }
                else { // Fall back to REGISTER INDEX if no constants available
                    instr.Rk = {RkType::REGISTER, reg_dist(rng)};
                }
            } 
            else { // REGISTER INDEX
                instr.Rk = {RkType::REGISTER, reg_dist(rng)};
            }
        }
    }

    void Input(double in) override {
        registers[1] = in;
    }

    double GetOutput() const override {
        return registers[0];
    }

    void ExecuteInstruction(Instruction const & instr) {
        // Instructions are represented as r[i] = r[j] op r[k]
        auto op_func = GLOBAL_OPERATORS.GetOperator(instr.op);
        double Rk_value;
        if (instr.Rk.first == RkType::CONSTANT) {
            Rk_value = std::get<double>(instr.Rk.second);
        }
        else if (instr.Rk.first == RkType::REGISTER) {
            Rk_value = registers[std::get<size_t>(instr.Rk.second)];
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

    void ResetRegisters() override {
        for (double & reg : registers) {
            reg = 0.0;
        }
    }

    void PrintProgram(std::ostream & os) const override {
        for (Instruction const & instr : instructions) {
            os << "r[" << instr.Ri << "] = r[" << instr.Rj << "] " << 
                GLOBAL_OPERATORS.GetOperatorName(instr.op) << " ";
            if (instr.Rk.first == RkType::REGISTER) {
                os << "r[" << std::get<size_t>(instr.Rk.second) << "]\n";
            }
            else if (instr.Rk.first == RkType::CONSTANT) {
                os << std::get<double>(instr.Rk.second) << "\n";
            }
        }
    }
};

#endif