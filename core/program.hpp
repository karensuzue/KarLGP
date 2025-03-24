#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <vector>
#include <string>
#include <variant>
#include <random>
#include <tuple>
#include <optional>

enum class RkType {
    REGISTER,
    CONSTANT
};

enum class RegisterType {
   NORMAL,
   READ_ONLY 
};

struct Instruction {
    // Instructions are representd as r[i] = r[j] op r[k]
    // Operators are determined using strings
    // Two possibilities for r[k] - register or constant.
    // Because we're using numbers to represent registers, we need a way to differentiate
    // Use a flag to differentiate between constant values and register ID
        // ('reg', 2): Register 2
        // ('const', 2): Constant 
    size_t Ri, Rj; // Index of registers
    std::pair<RkType, std::variant<size_t, double>> Rk; // 'size_t' for index, 'double' for constant
    size_t op; // Index of operator in global operators set
};

class Program {
private:
    int register_count;
    int program_length;
    double rk_prob {0.3}; // Hard-coded

    std::vector<double> registers; // Holds register values (ONLY DOUBLE FOR NOW)
    std::vector<RegisterType> register_types; // Controls register access

    std::vector<Instruction> instructions; // Program instructions

    std::optional<double> fitness;
    
    std::mt19937 rng; // Random number generator

public:
    Program() : register_count(REGISTER_COUNT), program_length(PROGRAM_LENGTH), rng(SEED) { 
        registers = std::vector<double>(register_count, 0);
        register_types = std::vector<RegisterType>(register_count, RegisterType::NORMAL);

        // r[1] holds the input
        // r[0] holds the return value
        register_types[1] = RegisterType::READ_ONLY;

        InitProgram();
        // EvalProgram();
    }

    Program(int rg_count, int prog_len) 
    : register_count(rg_count), program_length(prog_len), rng(SEED) {
        registers = std::vector<double>(register_count, 0);
        register_types = std::vector<RegisterType>(register_count, RegisterType::NORMAL);

        // r[1] holds the input
        // r[0] holds the return value
        register_types[1] = RegisterType::READ_ONLY;

        InitProgram();
    }

    bool IsEvaluated() const { return fitness.has_value(); }
    double GetFitness() const { 
        if (!fitness) throw std::runtime_error("Fitness has not been evaluated.");
        return fitness.value(); 
    }
    void SetFitness(double val) { fitness = val; }
    void ResetFitness() { fitness.reset(); }

    std::vector<Instruction> & GetInstructions() { return instructions; } // for mutation
    std::vector<Instruction> const & GetInstructions() const { return instructions; }
    void SetInstructions(std::vector<Instruction> & in) { instructions = in; }

    void InitProgram() {
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

    void Input(double in) {
        registers[1] = in;
    }

    double GetOutput() {
        return registers[0];
    }

    void ExecuteInstruction(Instruction const & instr) {
        // Instructions are representd as r[i] = r[j] op r[k]
        auto op_func = GLOBAL_OPERATORS.GetOperator(instr.op);
        double Rk_value;
        if (instr.Rk.first == RkType::CONSTANT) {
            Rk_value = std::get<double>(instr.Rk.second);
        }
        else if (instr.Rk.first == RkType::REGISTER) {
            Rk_value = registers[std::get<size_t>(instr.Rk.second)];
        }
        registers[instr.Ri] = op_func(registers[instr.Rj], Rk_value);
    }

    double ExecuteProgram() {
        for (Instruction const & instr : instructions) {
            ExecuteInstruction(instr);
        }
        return registers[0]; // output register
    }

    void ResetRegisters() {
        for (double & reg : registers) {
            reg = 0.0;
        }
    }

    void PrintProgram(std::ostream & os) const {
        for (Instruction const & instr : instructions) {
            os << "r[" << instr.Ri << "] = r[" << instr.Rj << "] " << 
                GLOBAL_OPERATORS.GetOperatorName(instr.op) << " ";
            if (instr.Rk.first == RkType::REGISTER) {
                os << "r[" << std::get<size_t>(instr.Rk.second) << "]\n";
            }
            else if (instr.Rk.first == RkType::CONSTANT) {
                os << std::get<double>(instr.Rk.second) << "\n";
            }
            os << "\n";
        }
    }

    friend std::ostream & operator<<(std::ostream & os, Program const & prog) {
        prog.PrintProgram(os);
        return os;
    }
};

#endif