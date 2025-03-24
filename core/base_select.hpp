#ifndef BASE_SELECT_HPP
#define BASE_SELECT_HPP

class Selector {
public:
    virtual ~Selector() = default;
    virtual Program const & Select(std::vector<Program> const & pop) const = 0;
};

#endif