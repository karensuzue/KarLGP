#ifndef ARITH_PROG_HPP
#define ARITH_PROG_HPP

// ARITHMETIC PROGRAMS CURRENTLY DON'T SUPPORT TERNARY OPERATORS OR NOVELTY

#include <vector>
#include <variant>
#include <random>
#include <optional>
#include <algorithm>
#include <unordered_set>

#include <sstream>

#include "base_prog.hpp"

class ArithmeticProgram : public Program {
private:
    // Evaluator * evaluator_ptr;

    size_t register_count;
    size_t program_length;
    double rk_prob {0.3}; // Hard-coded, probability of including constant over register

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

    // void SetEvaluator(Evaluator * evaluator) override { evaluator_ptr = evaluator; }

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

    void ResetRegisters() override {
        for (double & reg : registers) {
            reg = 0.0;
        }
    }

    std::vector<double> GetRegisters() const override {
        return registers;
    }

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

    // Calculates proportion of structural introns in a single program
    // Not sure if I should add this to the base class
    // This implementation is missing step 3 for control flow operations
        // Will be implemented once I figure out how to integrate those operations into the system
    double StructuralIntronProp() const override {
        // Start with output register
        std::unordered_set<size_t> effective_registers = {0}; 
        // All instructions start off 'unmarked'
        std::vector<bool> is_effective_instruct(instructions.size(), false); 

        // Go backwards through program
        for (int i {static_cast<int>(instructions.size() - 1)}; i >= 0; --i) {
            Instruction const & temp {instructions[i]};
            // If instruction writes to an effective register, mark it as effective
            if (effective_registers.contains(temp.Ri)) { 
                is_effective_instruct[i] = true;
                // Insert operand registers into the effective set if we haven't done so already
                if (!effective_registers.contains(temp.Rj)) {
                    effective_registers.insert(temp.Rj);
                }
                if (temp.Rk_type == RkType::REGISTER) {
                    size_t temp_Rk {std::get<size_t>(temp.Rk)};
                    if (!effective_registers.contains(temp_Rk)) {
                        effective_registers.insert(temp_Rk);
                    }
                }
            }
        }
        size_t effective_count {
            static_cast<size_t>(
                std::count(is_effective_instruct.begin(), is_effective_instruct.end(), true))};
        // Inverse to get non-effective instructions
        return 1.0 - static_cast<double>(effective_count) / program_length;
    }


    // Calculates proportion of semantic introns in a single program
    // Less accurate?
    double SemanticIntronProp(Evaluator const & eval) override {
        ResetRegisters(); // just in case

        std::vector<double> input_set {eval.GetInputSet()};
        size_t input_count {input_set.size()};

        // Vector of semantic vectors (same size as instruction set)
        // semantic_before[i][j] = value in Ri (destination) BEFORE executing instruction i on input j
        std::vector<std::vector<double>> semantic_before(program_length,
            std::vector<double>(input_count, 0.0));
        // semantic_after[i][j] = value in Ri (destination) AFTER executing instruction i on input j
        std::vector<std::vector<double>> semantic_after(program_length,
            std::vector<double>(input_count, 0.0));

        // Iterate through inputs
        for (size_t input_idx {0}; input_idx < input_count; ++input_idx) {
            ResetRegisters();
            Input(input_set[input_idx]);
            
            // Iterate through instructions
            for (size_t instruct_idx {0}; instruct_idx < program_length; ++instruct_idx) {
                // Get current instruction
                Instruction const & instruct {instructions[instruct_idx]};
                // BEFORE value of Ri (current instruction's destination register)
                semantic_before[instruct_idx][input_idx] = registers[instruct.Ri];
                ExecuteInstruction(instruct);
                // AFTER value of Ri
                semantic_after[instruct_idx][input_idx] = registers[instruct.Ri];
            }
        }

        // Compare before/after semantic vectors, look for matches
        size_t intron_count {0};
        for (size_t i {0}; i < program_length; ++i) {
            bool is_intron {true};
            for (size_t j {0}; j < input_count; ++j) {
                // Earlier, I used semantic_before[i][j] != semantic_after[i][j]
                // but this may have made the function too strict.
                if (std::abs(semantic_before[i][j] - semantic_after[i][j]) > 1e-6) {
                    is_intron = false;
                    break;
                } 
            }
            if (is_intron) ++intron_count;
        }

        return static_cast<double>(intron_count) / program_length;
    }

    // More accurate?
    double SemanticIntronProp_Elimination(Evaluator const & eval) const override {
        // Evaluate original program
        ArithmeticProgram clone {*this};
        double og_fitness {eval.Evaluate(clone)};
    
        size_t intron_count {0};
    
        for (size_t i {0}; i < program_length; ++i) {
            // Clone program and remove instruction i
            ArithmeticProgram modified {*this};
            std::vector<Instruction> modified_instrs {instructions};
            modified_instrs.erase(modified_instrs.begin() + i);
            modified.SetInstructions(modified_instrs);
    
            double new_fitness {eval.Evaluate(modified)};

            // If new and old fitnesses are the same, it's an intron
            if (std::abs(new_fitness - og_fitness) <= 1e-6) {
                ++intron_count;
            }
        }
        return static_cast<double>(intron_count) / program_length;
    }
    
};

#endif