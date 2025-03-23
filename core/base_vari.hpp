#ifndef BASE_VARI_HPP
#define BASE_VARI_HPP

class Variator {
public:
    virtual ~Variator() = default;
    // For mutation, applies to input program in-place
    virtual void Apply(Program &) const = 0;
    // For crossover, returns a new child from two parents
    virtual Program Apply(Program const &, Program const &) const = 0;

};

#endif