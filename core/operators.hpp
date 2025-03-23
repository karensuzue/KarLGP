#ifndef OPERATORS_HPP
#define OPERATORS_HPP

#include <utility>
#include <string>
#include <functional>

class Operators {
private:
    // All operators must take two parameters for syntax consistency.
    // For unary operators, we ignore the second argument. 
    using operator_func = std::function<double(double, double)>;
    std::vector<std::pair<std::string, operator_func>> operators;
    std::mt19937 mutable rng;

public:
    Operators() : rng(SEED) { 
        // Default operators
        RegisterOperator("+", [](double a, double b) { return a + b; });
        RegisterOperator("-", [](double a, double b) { return a - b; });
        RegisterOperator("*", [](double a, double b) { return a * b; });
        RegisterOperator("/", [](double a, double b) { return (b != 0) ? a / b : 1.0; }); // protected
    }

    void RegisterOperator(std::string const & name, operator_func func) {
        operators.push_back({name, func});
    }

    void RegisterUnaryOperator(std::string const & name, std::function<double(double)> func) {
        RegisterOperator(name, [func](double a, double) {
            return func(a); 
        });
    }
    
    size_t Size() const {
        return operators.size();
    }

    size_t GetRandomOpID() const {
        // Selects a random operator from the set
        if (operators.empty()) throw std::runtime_error("No operators available.");
        std::uniform_int_distribution<size_t> dist(0, operators.size() - 1);
        return dist(rng);
    }

    operator_func GetOperator(size_t id) const {
        if (id >= operators.size()) throw std::out_of_range("Invalid operator ID.");
        return operators[id].second;
    }

    std::string GetOperatorName(size_t id) const {
        if (id >= operators.size()) throw std::out_of_range("Invalid operator ID.");
        return operators[id].first;
    }

    friend std::ostream & operator<<(std::ostream & os, Operators const & operator_set) {
        for (auto & op : operator_set.operators) {
            os << op.first << ", ";
        }
        os << "\n";
        return os;
    }
    
};



#endif