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
    std::unique_ptr<Program> prototype; // to grab the right Program subclass

    std::unique_ptr<Program> best_program;
    std::vector<double> best_fitness_history;
    std::vector<double> avg_fitness_history;

    std::mt19937 rng;

    bool verbose;
    std::ostream & os;

public:
    Estimator(std::unique_ptr<Evaluator> eval, 
        std::vector<std::unique_ptr<Variator>> vars,
        std::unique_ptr<Selector> sel,
        std::unique_ptr<Program> prot,
        bool verbose=false,
        std::ostream & os=std::cout)
        : pop_size(POP_SIZE), gens(GENS),
        evaluator(std::move(eval)), 
        variators(std::move(vars)),
        selector(std::move(sel)),
        prototype(std::move(prot)),
        operators(GLOBAL_OPERATORS),
        constants(GLOBAL_CONSTANTS),
        rng(SEED),
        verbose(verbose),
        os(os) {}
    
    Program const & GetBestProgram() const { return * best_program; }

    void InitPopulation() {
        for (size_t i {0}; i < pop_size; ++i) {
            std::unique_ptr<Program> prog {prototype->New()};
            population.push_back(std::move(prog));
        }
    }

    void EvalPopulation() {
        for (std::unique_ptr<Program> & p : population) {
            p->SetFitness(evaluator->Evaluate(*p));
        }
    }

    double AvgFitness() {
        double total {0};
        for (std::unique_ptr<Program> const & p : population) {
            total += p->GetFitness();
        }
        return total / population.size();
    }


    void Evolve() { 
        if (verbose) { PrintRunParam(os); }

        InitPopulation();
        EvalPopulation();

        best_program = population[0]->Clone();
        for (std::unique_ptr<Program> const & p : population) {
            if (p->GetFitness() < best_program->GetFitness()) {
                best_program = p->Clone();
            }
        }
        best_fitness_history.push_back(best_program->GetFitness());
        avg_fitness_history.push_back(AvgFitness());
        
        // Begin evolutionary loop
        for (size_t gen {0}; gen < gens; ++gen) {
            if (verbose) { PrintGenSummary(gen, best_fitness_history[gen], avg_fitness_history[0], os); }

            if (verbose && gen % 10 == 0) {
                os << "First 5 fitnesses: ";
                for (int i {0}; i < 5; ++i) os << population[i]->GetFitness() << " ";
                os << "\n";
            }

            std::vector<std::unique_ptr<Program>> new_pop;

            // Produce children
            while (new_pop.size() < pop_size) {
                Program const & parent1 {selector->Select(population)};
                Program const & parent2 {selector->Select(population)};

                std::unique_ptr<Program> child {parent1.Clone()}; // Default: copy parent1
                // Not sure if this is a good way
                // Be careful with ordering of variators in set
                // If no binary variator exists, we just mutate parent1
                for (std::unique_ptr<Variator> & variator: variators) {
                    if (variator->Type() == VariatorType::BINARY) {
                        child = variator->Apply(parent1, parent2);
                    }
                    else if (variator->Type() == VariatorType::UNARY) {
                        child = variator->Apply(*child);
                    }
                }
                new_pop.push_back(std::move(child));
            }

            // Update population, re-evaluate fitness, overall best program
            population = std::move(new_pop);
            EvalPopulation();

            for (std::unique_ptr<Program> const & p : population) {
                if (p->GetFitness() < best_program->GetFitness()) {
                    best_program = p->Clone();
                }
            }
            best_fitness_history.push_back(best_program->GetFitness());
            avg_fitness_history.push_back(AvgFitness());
        }
        
        if (verbose) {
            os << "\nEvolution complete (^_^)!\nOverall Best Fitness: " << best_program->GetFitness() << "\n";
        }
    }

    void PrintRunParam(std::ostream & os) const {
        os << "Seed: " << SEED << "\n"
            << "Population Size: " << POP_SIZE << "\n"
            << "Generations: " << GENS << "\n"
            << "Register Count: " << REGISTER_COUNT << "\n"
            << "Program Length: " << PROGRAM_LENGTH << "\n";
    }

    void PrintGenSummary(size_t gen, double b_f, double a_f, std::ostream & os) const {
        os << "Generation " << gen << " | Best Fit: " << b_f 
            << " | Avg Fit: " << a_f << "\n";
    }

    void ExportFitnessHistory() const {


    }
};

#endif