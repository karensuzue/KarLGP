#ifndef TOUR_SELECT_HPP
#define TOUR_SELECT_HPP

#include <random>
#include <memory>

#include "../core/base_select.hpp"

class TournamentSelect : public Selector {
private:
    size_t tournament_size;
    std::mt19937 mutable rng;
public:
    TournamentSelect() : tournament_size(TOUR_SIZE), rng(SEED) { }
    TournamentSelect(size_t tour_size) : tournament_size(tour_size), rng(SEED) { }

    Program const & Select(std::vector<std::unique_ptr<Program>> const & pop) const override {
        // Randomly pick 'tournament_size' individuals, return best one
        std::uniform_int_distribution<size_t> dist(0, pop.size() - 1);

        Program const * best {nullptr};
        for (size_t i {0}; i < tournament_size; ++i) {
            std::unique_ptr<Program> const & candidate {pop[dist(rng)]};
            if (!best || best->GetFitness() < candidate->GetFitness()) {
                best = &(*candidate); // address to the Program object under the smart pointer
            }
        }
        return *best;
    }
};

#endif