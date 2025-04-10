#ifndef ESTIMATOR_HPP
#define ESTIMATOR_HPP

#include <memory>
#include <random>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cassert>

class Estimator {
private:
    size_t pop_size {POP_SIZE};
    size_t gens {GENS};

    Operators operators;
    Constants constants;

    std::unique_ptr<Evaluator> evaluator;
    std::vector<std::unique_ptr<Variator>> variators;
    std::unique_ptr<Selector> selector;

    std::vector<std::unique_ptr<Program>> population;
    std::unique_ptr<Program> prototype; // to grab the right Program subclass

    std::unique_ptr<Program> best_program;
    std::vector<double> best_fitness_history; // NOTE: fitness is currently error, so minimizing
    std::vector<double> avg_fitness_history;
    std::vector<double> median_fitness_history;

    // Effect histories are one element smaller than fitness histories
    std::vector<double> quality_gain_history; 
    std::vector<double> success_rate_history;

    std::vector<double> semantic_intron_history;
    std::vector<double> semantic_intron_elim_history;
    std::vector<double> structural_intron_history;

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
      : operators(GLOBAL_OPERATORS),
        constants(GLOBAL_CONSTANTS),
        evaluator(std::move(eval)), 
        variators(std::move(vars)),
        selector(std::move(sel)),
        prototype(std::move(prot)),
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

    // // Proportion of structural introns in program code, averaged across the entire population
    double AvgStructuralIntronProp() {
        double intron_prop_sum {0};
        for (std::unique_ptr<Program> & p : population) {
            intron_prop_sum += p->StructuralIntronProp();
        }
        return intron_prop_sum / pop_size;
    }

    // First method (less accurate?)
    // Looks for differences in individual registers before and after executing an instruction
    double AvgSemanticIntronProp() {
        double intron_prop_sum {0};
        for (std::unique_ptr<Program> & p : population) {
            intron_prop_sum += p->SemanticIntronProp(*evaluator);
        }
        return intron_prop_sum / pop_size;
    }

    // Second method (more accurate?)
    // Eliminates instructions one by one to check which ones affect the final output (or fitness)
    double AvgSemanticIntronProp_Elimination() {
        double intron_prop_sum {0};
        for (std::unique_ptr<Program> & p : population) {
            intron_prop_sum += p->SemanticIntronProp_Elimination(*evaluator);
        }
        return intron_prop_sum / pop_size;
    }

    double AvgFitness() {
        double total {0};
        for (std::unique_ptr<Program> const & p : population) {
            total += p->GetFitness();
        }
        return total / population.size();
    }

    double MedianFitness() {
        // std::vector<std::unique_ptr<Program>> pop_ptr_copy;
        std::vector<double> fitnesses;
        for (size_t i {0}; i < pop_size; ++i) {
            // pop_ptr_copy.push_back(population[i]->Clone());
            fitnesses.push_back(population[i]->GetFitness());
        }

        std::sort(fitnesses.begin(), fitnesses.end());

        size_t mid {fitnesses.size() / 2};
        if (fitnesses.size() % 2 == 0) {
            return (fitnesses[mid - 1] + fitnesses[mid]) / 2;
        }
        else {
            return fitnesses[mid];
        }
        // std::sort(pop_ptr_copy.begin(), pop_ptr_copy.end(), [](auto const & a, auto const & b) {
        //     return a->GetFitness() < b->GetFitness();
        // });

        // for (size_t i {0}; i < pop_size; ++i) {
        //     std::cout << pop_ptr_copy[i]->GetFitness() << ", ";
        // }
        // std::cout << std::endl;
        
        return 0;
        
    }

    void Reset() {
        population.clear();
        best_program.reset();

        best_fitness_history.clear();
        avg_fitness_history.clear();
        median_fitness_history.clear();

        // quality_gain_history.clear();
        // success_rate_history.clear();

        semantic_intron_history.clear();
        semantic_intron_elim_history.clear();
        structural_intron_history.clear();
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
        median_fitness_history.push_back(MedianFitness());

        semantic_intron_history.push_back(AvgSemanticIntronProp());
        semantic_intron_elim_history.push_back(AvgSemanticIntronProp_Elimination());
        structural_intron_history.push_back(AvgStructuralIntronProp());
        
        // Begin evolutionary loop
        for (size_t gen {0}; gen < gens; ++gen) {
            if (verbose) { 
                PrintGenSummary(gen, best_fitness_history[gen], avg_fitness_history[gen], median_fitness_history[gen], os); 
            }

            if (verbose && gen % 10 == 0) {
                os << "First 5 fitnesses: ";
                for (int i {0}; i < 5; ++i) os << population[i]->GetFitness() << " ";
                os << "\n";
            }

            std::vector<std::unique_ptr<Program>> new_pop;
            // int success_count {0}; // For measuring success rate

            // Produce children - by default, we replace the entire population
            while (new_pop.size() < pop_size) {
                Program const & parent1 {selector->Select(population)};
                Program const & parent2 {selector->Select(population)};

                std::unique_ptr<Program> child {parent1.Clone()}; // Default: copy parent1

                // Not sure if this is a good way
                // Be careful with ordering of variators in set
                // If no binary variator exists, we just mutate parent1

                // bool two_parents {false}; // For measuring success rate
                for (std::unique_ptr<Variator> & variator: variators) {
                    if (variator->Type() == VariatorType::BINARY) {
                        child = variator->Apply(parent1, parent2);
                        // two_parents = true;
                    }
                    else if (variator->Type() == VariatorType::UNARY) {
                        child = variator->Apply(*child);
                    }
                }

                // Measuring success rate
                // double parent1_fitness {parent1.GetFitness()};
                // double parent2_fitness {parent2.GetFitness()};
                // double child_fitness {evaluator->Evaluate(*child)};

                // bool success {false};
                // if (two_parents) { 
                //     // Better fitness = lower fitness, in this case
                //     // Not sure if I should switch to using explicit improvement over both parents
                //     // I assume using "or" is more realistic and captures partial improvements
                //     success = child_fitness < parent1_fitness || child_fitness < parent2_fitness;
                // }
                // else {
                //     success = child_fitness < parent1_fitness;
                // }
                // if (success) ++success_count;


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

            // double prev_avg_fitness {avg_fitness_history.back()};
            double curr_avg_fitness {AvgFitness()};
            // Inverse, makes more sense for quality gain to increase
            // double quality_gain {-(curr_avg_fitness - prev_avg_fitness)}; 
            avg_fitness_history.push_back(curr_avg_fitness);
            // quality_gain_history.push_back(quality_gain);

            // success_rate_history.push_back(static_cast<double>(success_count) / static_cast<double>(pop_size));

            median_fitness_history.push_back(MedianFitness());

            semantic_intron_history.push_back(AvgSemanticIntronProp());
            semantic_intron_elim_history.push_back(AvgSemanticIntronProp_Elimination());
            structural_intron_history.push_back(AvgStructuralIntronProp());
        }
        
        if (verbose) {
            os << "\nEvolution complete (^_^)!\nOverall Best Fitness: " << best_program->GetFitness() << "\n";
        }
        // ExportFitnessHistory();
    }

    void MultiRunEvolve(int run_count=10, std::ostream & os=std::cout) {
        verbose = false; // just in case

        for (int i {0}; i < run_count; ++i) {
            rng.seed(i);
            Reset();
            Evolve();
            ExportFitnessHistory("fitness_run_" + std::to_string(i) + ".csv");
            // ExportEffectHistory("effect_run_" + std::to_string(i) + ".csv");
            ExportIntronHistory("intron_run_" + std::to_string(i) + ".csv");
            
            std::ofstream ofs("best_program_" + std::to_string(i) + ".txt");
            if (ofs.is_open()) {
                ofs << best_program->GetFitness() << "\n";
                ofs << *best_program;
            }

            os << "Finished run " << i << "\n";
        }        
    }

    void PrintRunParam(std::ostream & os) const {
        os << "Seed: " << SEED << "\n"
            << "Population Size: " << POP_SIZE << "\n"
            << "Generations: " << GENS << "\n"
            << "Register Count: " << REGISTER_COUNT << "\n"
            << "Program Length: " << PROGRAM_LENGTH << "\n";
    }

    void PrintGenSummary(size_t gen, double b_f, double a_f, double m_f, std::ostream & os) const {
        os << "Generation " << gen << " | Best Fit: " << b_f 
            << " | Avg Fit: " << a_f
            << " | Median Fit: " << m_f << "\n";
    }

    void ExportFitnessHistory(std::string const & filename="fitness_history.csv") const {
        assert(best_fitness_history.size() == avg_fitness_history.size());
        assert(avg_fitness_history.size() == median_fitness_history.size());

        std::ofstream ofs(filename);
        if (ofs.is_open()) {
            ofs << "Generation,BestFitness,MedianFitness,AvgFitness\n";
            ofs << std::fixed << std::setprecision(6);
            for (size_t i {0}; i < best_fitness_history.size(); ++i) {
                ofs << i << "," 
                    << best_fitness_history[i] << ","
                    << median_fitness_history[i] << ","
                    << avg_fitness_history[i] << "\n";
            }
            ofs.close();
        }
    }

    void ExportEffectHistory(std::string const & filename="effect_history.csv") const {
        assert(quality_gain_history.size() == success_rate_history.size());
        std::ofstream ofs(filename);
        if (ofs.is_open()) {
            ofs << "Generation,QualityGain,SuccessRate\n";
            ofs << std::fixed << std::setprecision(6);
            for (size_t i {0}; i < quality_gain_history.size(); ++i) {
                ofs << i+1 << ","
                    << quality_gain_history[i] << ","
                    << success_rate_history[i] << "\n";
            }
        }
    }

    void ExportIntronHistory(std::string const & filename="intron_history.csv") const {
        assert(semantic_intron_history.size() == structural_intron_history.size());
        std::ofstream ofs(filename);
        if (ofs.is_open()) {
            ofs << "Generation,Semantic,SemanticElim, Structural\n";
            ofs << std::fixed << std::setprecision(6);
            for (size_t i {0}; i < semantic_intron_history.size(); ++i) {
                ofs << i << ","
                    << semantic_intron_history[i] << ","
                    << semantic_intron_elim_history[i] << ","
                    << structural_intron_history[i] << "\n";
            }
        }
    }
};

#endif