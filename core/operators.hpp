#ifndef OPERATORS_HPP
#define OPERATORS_HPP

#include <cassert>
#include <utility>
#include <string>
#include <functional>

#include "emp/base/vector.hpp"

class Operators {
private:
    // Unless ternary is set, all operators take two parameters for syntax consistency.
    // For unary operators, we ignore the second argument. 
    using operator_func = std::function<double(double, double)>;
    std::vector<std::pair<std::string, operator_func>> operators;
    
    // Ternary is set to false by default
    using ternary_operator_func = std::function<double(double, double, double)>;
    emp::vector<std::pair<std::string, ternary_operator_func>> ternary_operators;

    std::mt19937 mutable rng;

    bool ternary;

public:
    Operators(bool tern=false) : rng(SEED), ternary(tern) { 
        // Default operators
        RegisterOperator("ADD", [](double a, double b) { return a + b; });
        RegisterOperator("SUB", [](double a, double b) { return a - b; });
        RegisterOperator("MULT", [](double a, double b) { return a * b; });
        RegisterOperator("DIV", [](double a, double b) { return (b != 0) ? a / b : 1.0; }); // protected

        RegisterLogicOperators();
        
        if (ternary) { // 3 operands allowed (mostly for a 3-arg conditional)
            RegisterTernaryOperators();
        }
    }

    void RegisterOperator(std::string const & name, operator_func func) {
        operators.emplace_back(name, func);
    }

    void RegisterUnaryOperator(std::string const & name, std::function<double(double)> func) {
        RegisterOperator(name, [func](double a, double) {
            return func(a); 
        });
    }

    void RegisterTernaryOperator(std::string const & name, ternary_operator_func func) {
        ternary_operators.emplace_back(name, func);
    }
    

    // Helper function for logic operators
    bool static AsBool(double val) {
        // Too strict?
        // return std::abs(val) > 1e-6; 

        // If negative, count as false
        return val > 0.0;
    }

    // Bundle a set of default logic operators
    void RegisterLogicOperators() {
        RegisterOperator("AND", [](double a, double b) { return AsBool(a) && AsBool(b); });
        RegisterOperator("OR", [](double a, double b) { return AsBool(a) || AsBool(b); });
        RegisterOperator("NAND", [](double a, double b) { return !(AsBool(a) && AsBool(b)); });
        RegisterOperator("NOR", [](double a, double b) { return !(AsBool(a) || AsBool(b)); });
        RegisterOperator("XOR", [](double a, double b) { return AsBool(a) != AsBool(b); });

        RegisterOperator("GREATER", [](double a, double b) { return AsBool(a) > AsBool(b); });
        RegisterOperator("EQUAL", [](double a, double b) { return AsBool(a) == AsBool(b); });
        RegisterOperator("LESS", [](double a, double b) { return AsBool(a) < AsBool(b); });

        // Let b pass if a is true (if a, then b)
        RegisterOperator("IF", [](double a, double b) {
            return AsBool(a)? b : 0.0;
        });

        RegisterUnaryOperator("NOT", [](double a) { return !AsBool(a); });
    }

    // Bundle a set of default ternary operators
    void RegisterTernaryOperators() {
        RegisterTernaryOperator("IF-3", [](double cond, double a, double b) {
            return AsBool(cond)? a : b;
        }); 
    }
    
    size_t Size() const {
        return operators.size();
    }

    size_t GetRandomOpID() const {
        // Selects a random operator from the set
        assert(!operators.empty() && "No operators available.");
        std::uniform_int_distribution<size_t> dist(0, operators.size() - 1);
        return dist(rng);
    }

    size_t GetRandomTernaryOpID() const {
        assert(!ternary_operators.empty() && "No ternary operators available.");
        std::uniform_int_distribution<size_t> dist(0, ternary_operators.size() - 1);
        return dist(rng);
    }

    operator_func GetOperator(size_t id) const {
        assert(id < operators.size() && "Invalid operator ID.");
        return operators[id].second;
    }

    std::string GetOperatorName(size_t id) const {
        assert(id < operators.size() && "Invalid operator ID.");
        return operators[id].first;
    }


    // TERNARY-ONLY FUNCTIONS

    bool IsSetTenary() const {
        return ternary;
    }

    ternary_operator_func GetTernaryOperator(size_t id) const {
        assert(id < ternary_operators.size() && "Invalid ternary operator ID.");
        return ternary_operators[id].second;
    }

    std::string GetTernaryOperatorName(size_t id) const {
        assert(id < ternary_operators.size() && "Invalid ternary operator ID.");
        return ternary_operators[id].first;
    }

    friend std::ostream & operator<<(std::ostream & os, Operators const & operator_set) {
        for (auto & op : operator_set.operators) {
            os << op.first << ", ";
        }
        os << "\n";

        if (operator_set.ternary) {
            for (auto & op : operator_set.ternary_operators) {
                os << op.first << ", ";
            }
            os << "\n";
        }
        return os;
    }
};

#endif