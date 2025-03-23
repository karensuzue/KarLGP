#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <iostream>
#include <vector>
#include <random>

class Constants {
private:
    // std::vector<int> int_constants;
    // std::vector<double> dec_constants;
    std::vector<double> constants;
    std::mt19937 mutable rng;

public:
    Constants() : rng(SEED) { }

    // void RegisterConstant(int num) {
    //     int_constants.push_back(num);
    // }

    // void RegisterConstant(double num) {
    //     dec_constants.push_back(num);
    // }

    // size_t IntSetSize() const {
    //     return int_constants.size();
    // }
    
    // size_t DecSetSize() const {
    //     return dec_constants.size();
    // }

    void RegisterConstant(double num) {
        constants.push_back(num);
    }

    size_t Size() const {
        return constants.size();
    }


    // friend std::ostream & operator<<(std::ostream & os, Constants const & constant_set) {
    //     os << "Integer Constants: ";
    //     for (int c : constant_set.int_constants) {
    //         os << c << " ";
    //     }
    //     os << "\nDecimal Constants: ";
    //     for (double c : constant_set.dec_constants) {
    //         os << c << " ";
    //     }
    //     os << "\n";

    //     return os;
    // }

    friend std::ostream & operator<<(std::ostream & os, Constants const & constant_set) {
        for (double c : constant_set.constants) {
            os << c << " ";
        }
        os << "\n";
        return os;
    }

    // int GetRandomIntConstant() {
    //     if (int_constants.empty()) throw std::runtime_error("No integer constants available.");
    //     std::uniform_int_distribution<size_t> dist(0, int_constants.size() - 1);
    //     return int_constants[dist(rng)];

    // }

    // double GetRandomDecConstant() {
    //     if (dec_constants.empty()) throw std::runtime_error("No decimal constants available.");
    //     std::uniform_int_distribution<size_t> dist(0, dec_constants.size() - 1);
    //     return dec_constants[dist(rng)];
    // }

    double GetRandomConstant() const {
        if (constants.empty()) throw std::runtime_error("No constants available.");
        std::uniform_int_distribution<size_t> dist(0, constants.size() - 1);
        return constants[dist(rng)];
    }

};

#endif