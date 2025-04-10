#ifndef BASE_EVAL_HPP
#define BASE_EVAL_HPP

class Program;

class Evaluator {
public:
    virtual ~Evaluator() = default;
    // Must implement in subclasses
    virtual std::vector<double> GetInputSet() const = 0;
    virtual double Evaluate(Program & ) const = 0;
};

#endif