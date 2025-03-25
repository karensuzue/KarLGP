#ifndef BASE_SELECT_HPP
#define BASE_SELECT_HPP

#include <memory>

class Selector {
public:
    virtual ~Selector() = default;
    virtual Program const & Select(std::vector<std::unique_ptr<Program>> const & pop) const = 0;
};

#endif