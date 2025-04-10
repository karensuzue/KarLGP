#ifndef RAND_VARI_HPP
#define RAND_VARI_HPP

#include <random>
#include <memory>

#include "../core/base_vari.hpp"

class RandomVariator: public Variator {
private:
    std::mt19937 mutable rng;

public:
    RandomVariator() { }

    VariatorType Type() const override { return VariatorType::UNARY; }
    // For mutation, applies to input program in-place
    std::unique_ptr<Program> Apply(Program const & prog) const override {
        std::unique_ptr<Program> child = prog.New();
        return child;
    }
    
    std::unique_ptr<Program> Apply(Program const &, Program const &) const override {
        throw std::runtime_error("RandomVariator is not a binary operator.");
    }
};

#endif