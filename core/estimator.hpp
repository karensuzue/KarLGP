#ifndef ESTIMATOR_HPP
#define ESTIMATOR_HPP

#include <memory>
#include <random>
#include <iostream>

class Estimator {
private:
    size_t pop_size, gens;

    Operators operators;
    Constants constants;

    // C++ requires pointers or references for polymorphism?
    std::unique_ptr<Evaluator> evaluator;
    std::vector<std::unique_ptr<Variator>> variators;
    std::unique_ptr<Selector> selector;

    std::vector<std::unique_ptr<Program>> population;

    std::unique_ptr<Program> best_program;
    std::vector<double> best_fitness_history;

    std::mt19937 rng;

    bool verbose;
    std::ostream & os;

public:
    Estimator(std::unique_ptr<Evaluator> eval, 
        std::vector<std::unique_ptr<Variator>> vars,
        std::unique_ptr<Selector> sel,
        bool verbose=false,
        std::ostream & os=std::cout)
        : pop_size(POP_SIZE), gens(GENS),
        evaluator(std::move(eval)), 
        variators(std::move(vars)),
        selector(std::move(sel)),
        operators(GLOBAL_OPERATORS),
        constants(GLOBAL_CONSTANTS),
        rng(SEED),
        verbose(verbose),
        os(os) {}
    
    Program GetBestProgram() const { return best_program; }

    void InitPopulation() {
        for (size_t i {0}; i < pop_size; ++i) {
            // std::unique_ptr<Program> prog;
            // population.push_back(prog);
            population.emplace_back(prog);
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
        best_fitness_history.push_back(best_program.GetFitness());
        
        // Begin evolutionary loop
        for (size_t gen {0}; gen < gens; ++gen) {
            if (verbose) { PrintGenSummary(gen, os); }
            std::vector<Program> new_pop;

            // Produce children
            while (new_pop.size() < pop_size) {
                Program parent1 {selector->Select(population)};
                Program parent2 {selector->Select(population)};

                Program child {parent1}; // Default: copy parent1
                // Not sure if this is a good way
                // Be careful with ordering of variators in set
                // If no binary variator exists, we just mutate parent1
                for (std::unique_ptr<Variator> & variator: variators) {
                    if (variator->Type() == VariatorType::BINARY) {
                        child = variator->Apply(parent1, parent2);
                    }
                    else if (variator->Type() == VariatorType::UNARY) {
                        child = variator->Apply(child);
                    }
                }
                new_pop.push_back(child);
            }

            // Update population, re-evaluate fitness, overall best program
            population = std::move(new_pop);
            EvalPopulation();
            Program gen_best = *std::min_element(population.begin(), population.end(),
                [](Program const & a, Program const & b) {
                    return a.GetFitness() < b.GetFitness();
                });
            best_fitness_history.push_back(gen_best.GetFitness());
            if (gen_best.GetFitness() < best_program.GetFitness()) {
                best_program = gen_best;
            }
        }
        
        if (verbose) {
            os << "\nEvolution complete (^_^)!\nOverall Best Fitness: " << best_program.GetFitness() << "\n";
        }
    }

    void PrintRunParam() const {

    }

    void PrintGenSummary(size_t gen, std::ostream & os) const {
        Program gen_best = *std::min_element(population.begin(), population.end(),
            [](Program const & a, Program const & b) {
                return a.GetFitness() < b.GetFitness();
            });
        os << "Generation " << gen << " | Gen Best Fit: " << gen_best.GetFitness()
            << " | Overall Best Fit: " << best_program.GetFitness() << "\n";
    }

    void ExportFitnessHistory() const {


    }
};

#endif