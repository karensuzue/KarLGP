#ifndef BASE_EVAL_HPP
#define BASE_EVAL_HPP


class Evaluator {
public:
    virtual ~Evaluator() = default;
    // Must implement in subclasses
    virtual double Evaluate(Program & ) const = 0;
};

#endif