#ifndef ESTIMATOR_HPP
#define ESTIMATOR_HPP

#include <memory>
#include <random>

class Estimator {
private:
    size_t pop_size, gens;
    // C++ requires pointers or references for polymorphism?
    std::unique_ptr<Evaluator> evaluator;
    std::vector<std::unique_ptr<Variator>> variators;
    std::unique_ptr<Selector> selector;

    std::vector<Program> population;
    Program best_program;

    std::mt19937 rng;

    bool verbose;

public:
    Estimator(size_t pop_size, size_t gens, 
        std::unique_ptr<Evaluator> eval, 
        std::vector<std::unique_ptr<Variator>> vars,
        std::unique_ptr<Selector> sel,
        bool verbose=false)
        : pop_size(pop_size), gens(gens),
        evaluator(std::move(eval)), 
        variators(std::move(vars)),
        selector(std::move(sel)),
        rng(SEED),
        verbose(verbose) {}
    
    Program GetBestProgram() const { return best_program; }

    void InitPopulation() {
        for (size_t i {0}; i < pop_size; ++i) {
            // Since the population could be large, avoid copy/move overhead of push_back()
            population.emplace_back();
        }
    }

    void EvalPopulation() {
        for (Program & p : population) {
            p.SetFitness(evaluator->Evaluate(p));
        }
    }

    void Evolve() { 
        InitPopulation();
        EvalPopulation();

        // Fitness is error-based, so minimize
        best_program = *std::min_element(population.begin(), population.end(),
            [](Program const & a, Program const & b) {
                return a.GetFitness() < b.GetFitness();
            });
        
        for (size_t gen {0}; gen < gens; ++gen) {
            std::vector<Program> new_pop;
            while (new_population.size() < pop_size) {
                Program parent1 {selector->Select(population)};
                Program parent2 {selector->Select(population)};

                Program child; 
                for (std::unique_ptr<Variator> & variator: variators) {
                    if (variator->Type() == VariatorType::UNARY) {
                        child = variator->Apply(parent1); // Default: mutate parent1
                    }
                    else if (variator->Type() == VariatorType::BINARY) {
                        child = variator->Apply(parent1, parent2);
                    }
                }
            }
            population = std::move(new_pop);
        }   
    }

};

#endif