#ifndef BASE_PROG_HPP
#define BASE_PROG_HPP

#include <memory>

#include "instructions.hpp"

class Program {
public:
    virtual ~Program() = default;

    // Necessary for polymorphism
    virtual std::unique_ptr<Program> Clone() const = 0;

    virtual void InitProgram() = 0;
    virtual double ExecuteProgram() = 0;
    virtual void Input(double x) = 0;
    virtual double GetOutput() const = 0;

    virtual double GetFitness() const = 0;
    virtual void SetFitness(double f) = 0;
    virtual bool IsEvaluated() const = 0;

    virtual void ResetRegisters() = 0;
    virtual void ResetFitness() = 0;

    virtual std::vector<Instruction> GetInstructions() const = 0;
    virtual void SetInstructions(std::vector<Instruction> const & instr) = 0;

    virtual void PrintProgram(std::ostream & os) const = 0;

    friend std::ostream & operator<<(std::ostream & os, Program const & prog) {
        prog.PrintProgram(os);
        return os;
    }
};

#endif