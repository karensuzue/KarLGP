#ifndef MAZE_PROG_HPP
#define MAZE_PROG_HPP

#include <cassert>
#include <memory>
#include <optional>
#include <fstream>

#include "emp/base/vector.hpp"

#include "../core/base_prog.hpp"

class MazeProgram : public Program {
private:
    // 5 input registers (for sensors) and 1 output register (movement - raw)
    size_t const min_register_count {6}; 
    size_t register_count, program_length;
    double rk_prob {0.3}; // Hard-coded, probability of including constant over register

    emp::vector<double> registers; // Holds register values (ONLY DOUBLE FOR NOW)
    // emp::vector<RegisterType> register_types; // Controls register access

    emp::vector<Instruction> instructions; // Program instructions

    std::optional<double> fitness; 

    std::optional<double> second_fitness; // Secondary fitness, does not effect selection

    // std::optional<double> novelty; // population-based metric
    // defined as the final position of the robot, averaged across multiple mazes
    std::optional<std::pair<double, double>> behavior; 
    
    std::mt19937 rng; // Random number generator

public:
    MazeProgram(size_t rc=REGISTER_COUNT, size_t pl=PROGRAM_LENGTH)
    : register_count(std::max(rc, min_register_count)),
      program_length(pl),
      rng(SEED)
    {   
        registers = emp::vector<double> (register_count, 0.0);
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
        ResetRegisters();
        
        instructions = emp::vector<Instruction>(program_length);

        std::uniform_real_distribution<double> prob_dist(0.0, 1.0);
        std::uniform_int_distribution<size_t> reg_dist(0, register_count - 1);

        for (Instruction & instr : instructions) {
            instr.Ri = reg_dist(rng);
            instr.Rj = reg_dist(rng);

            // UNARY/BINARY OPERATOR
            if (prob_dist(rng) < 0.5) {
                instr.op = GLOBAL_OPERATORS.GetRandomOpID();
                instr.op_type = 0;
            }
            // TERNARY OPERATOR
            else {
                instr.op = GLOBAL_OPERATORS.GetRandomTernaryOpID();
                instr.op_type = 1;
                instr.Rt = reg_dist(rng);
            }

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

    double GetRkValue(Instruction const & instr) const {
        double Rk_value;
        if (instr.Rk_type== RkType::CONSTANT) {
            Rk_value = std::get<double>(instr.Rk);
        }
        else { // if (instr.Rk_type == RkType::REGISTER) {
            Rk_value = registers[std::get<size_t>(instr.Rk)];
        }
        return Rk_value;
    }

    void ExecuteInstruction(Instruction const & instr) {
        // Instructions are represented as r[i] = op (r[j] r[k])
        // OR r[i] = op (r[j] r[t] r[k]) if TERNARY
        double Rk_value {GetRkValue(instr)};

        if (instr.op_type == 0) { 
            auto op_func = GLOBAL_OPERATORS.GetOperator(instr.op);
            // Registers are clamped to avoid under/overflow
            registers[instr.Ri] = std::clamp(op_func(registers[instr.Rj], Rk_value), -1e6, 1e6);
        }
        else { // IF TERNARY
            auto op_func = GLOBAL_OPERATORS.GetTernaryOperator(instr.op);

            assert(instr.Ri < registers.size());
            assert(instr.Rj < registers.size());
            assert(instr.Rt.value() < registers.size());
            assert(instr.Rt.has_value());

            registers[instr.Ri] = std::clamp(
                op_func(registers[instr.Rj], registers[instr.Rt.value()], Rk_value), -1e6, 1e6);
        }
    }


    // This returns the RAW output 
    double ExecuteProgram() override {
        for (Instruction const & instr : instructions) {
            ExecuteInstruction(instr);
        }
        return std::clamp(registers[5], -1e6, 1e6); // output register
    }


    void Input(emp::vector<double> inputs) {
        // Registers 0-4 hold sensor inputs
        // Register 5 hold output
        for (size_t i {0}; i < 5; ++i) {
            registers[i] = inputs[i];
        }
    }

    void Input(double ) override {
        assert(false && "Single input is not an option for MazeProgram.");
    }

    // This returns the RAW output
    double GetOutput() const override {
        return registers[5]; // raw
    }

    // This returns the PROCESSED output 
    int GetOutputStep() const { 
        return static_cast<int>(std::round(registers[5])) % 4; // 4 available actions 
    }

    double GetFitness() const override {
        assert(fitness && "Fitness has not been evaluated.");
        return fitness.value(); 
    }

    double GetSecondFitness() const override {
        assert(second_fitness && "Secondary fitness has not been evaluated.");
        return second_fitness.value(); 
    }

    std::pair<double, double> GetBehavior() const {
        assert(behavior.has_value() && "Behavior has not been evaluated.");
        return behavior.value(); 
    }

    double GetBehaviorR() const {
        assert(behavior.has_value() && "Behavior has not been evaluated.");
        return behavior.value().first;
    }
    
    double GetBehaviorC() const {
        assert(behavior.has_value() && "Behavior has not been evaluated.");
        return behavior.value().second;
    }

    // double GetNovelty() const {
    //     assert(novelty.has_value() && "Novelty has not been evaluated.");
    //     return novelty.value();
    // }

    void SetFitness(double val) override { fitness = val; }
    void SetBehavior(std::pair<double, double> val) { behavior = val; }
    void SetSecondFitness(double val) override { second_fitness = val; }
    // void SetNovelty(double val) { novelty = val; }

    bool IsEvaluated() const override { return fitness.has_value(); }
    bool IsBehaviorEvaluated() const { return behavior.has_value(); }
    bool IsSecondEvaluated() const override { return second_fitness.has_value(); }
    // bool IsNoveltyEvaluated() const { return novelty.has_value(); }

    void ResetFitness() override { fitness.reset(); }
    void ResetSecondFitness() override { second_fitness.reset(); }
    void ResetBehavior() { behavior.reset(); }
    // void ResetNovelty() { novelty.reset(); }

    void ResetRegisters() override {
        for (double & reg : registers) {
            reg = 0.0;
        }
    }

    emp::vector<double> GetRegisters() const override {
        return registers;
    }

    emp::vector<Instruction> GetInstructions() const override { return instructions; }
    void SetInstructions(emp::vector<Instruction> const & in) override { instructions = in; }

    void PrintProgram(std::ostream & os) const override {
        for (Instruction const & instr : instructions) {
            os << "r[" << instr.Ri << "] = ";

            if (instr.op_type == 0) os << GLOBAL_OPERATORS.GetOperatorName(instr.op);
            else os << GLOBAL_OPERATORS.GetTernaryOperatorName(instr.op);
            
            os << " (r[" << instr.Rj << "], ";

            if (instr.op_type == 1) {
                assert(instr.Rt.has_value());
                os << "r[" << instr.Rt.value() << "], ";
            }

            if (instr.Rk_type == RkType::REGISTER) {
                os << "r[" << std::get<size_t>(instr.Rk) << "])\n";
            }
            else if (instr.Rk_type == RkType::CONSTANT) {
                os << std::get<double>(instr.Rk) << ")\n";
            }
        }
    }


    void LoadMazeProgram(std::string const & filename) {
        std::ifstream ifs(filename);
        assert(ifs.is_open());
        
        std::vector<Instruction> instrs;
        std::string line;
        while (std::getline(ifs, line)) {
            if (line.empty()) continue;

            std::stringstream ss(line);
            Instruction instr;
            
            // Parse (binary): r[i] = OP (r[j] r[k])
            // Parse (ternary): r[i] = OP (r[j], r[t], R[k]/constant)
            std::string token;
            std::getline(ss, token, '['); // "r"
            std::getline(ss, token, ']'); 
            // std::cout << "i = " << token << std::endl;
            instr.Ri = std::stoi(token);
    
            std::getline(ss, token, '('); // " = OP"
            std::istringstream token_stream(token);
            std::string equals, op_name;
            token_stream >> equals >> op_name;
            // std::cout << "operator name = " << op_name << std::endl;
    
            instr.op_type = GLOBAL_OPERATORS.IsTernaryOperator(op_name) ? 1 : 0;
            // std::cout << "type of operator = " << instr.op_type << std::endl;
            instr.op = instr.op_type ? GLOBAL_OPERATORS.GetTernaryOperatorID(op_name)
                                     : GLOBAL_OPERATORS.GetOperatorID(op_name);
            // std::cout << "operator id = " << instr.op << std::endl;
    
            std::getline(ss, token, '[');
            std::getline(ss, token, ']'); 
            // std::cout << "j = " << token << std::endl;
            instr.Rj = std::stoi(token);
    
            if (instr.op_type == 1) {
                std::getline(ss, token, '[');
                std::getline(ss, token, ']'); 
                // std::cout << "t = " << token << std::endl;
                instr.Rt = std::stoi(token);
            }
    
            // Parse r[k] or constant
            std::getline(ss, token, ')');
            std::cout << token << std::endl;
            std::stringstream final(token);
            final >> token >> token;
            std::cout << token << std::endl;

            if (token.find("r[") != std::string::npos) {
                size_t start = token.find('[') + 1;
                size_t end = token.find(']');
                instr.Rk_type = RkType::REGISTER;
                std::cout << "k = " << token.substr(start, end-start) << std::endl;
                instr.Rk = std::stoul(token.substr(start, end - start));
            } else {
                instr.Rk_type = RkType::CONSTANT;
                instr.Rk = std::stod(token);
            }
            instrs.emplace_back(std::move(instr));
        }
    
        SetInstructions(instrs);
        // SetFitness(fitness_val);
    }

    

    // UNIMPLEMENTED
    double StructuralIntronProp() const override { return 0; }
    double SemanticIntronProp(Evaluator const & ) override { return 0; }
    double SemanticIntronProp_Elimination(const Evaluator& ) const override { return 0; }
};

#endif