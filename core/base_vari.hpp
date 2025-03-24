#ifndef BASE_VARI_HPP
#define BASE_VARI_HPP

enum class VariatorType {
    BINARY, // crossover
    UNARY // mutation
};

class Variator {
public:
    virtual ~Variator() = default;
    virtual VariatorType Type() const = 0;
    // For mutation, applies to input program in-place
    virtual Program Apply(Program const &) const = 0;
    // For crossover, returns a new child from two parents
    virtual Program Apply(Program const &, Program const &) const = 0;

};

#endif