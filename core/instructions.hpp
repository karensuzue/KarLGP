#ifndef INSTRUCTIONS_HPP
#define INSTRUCTIONS_HPP

#include <variant>

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


#endif