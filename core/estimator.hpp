#ifndef ESTIMATOR_HPP
#define ESTIMATOR_HPP

#include <memory>
#include <random>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cassert>

#include "emp/base/vector.hpp"

class Estimator {
private:
    size_t pop_size {POP_SIZE};
    size_t gens {GENS};
    size_t elitism_count {ELITISM_COUNT};

    Operators operators;
    Constants constants;

    std::unique_ptr<Evaluator> evaluator;
    emp::vector<std::unique_ptr<Variator>> variators;
    std::unique_ptr<Selector> selector;

    emp::vector<std::unique_ptr<Program>> population;
    std::unique_ptr<Program> prototype; // to grab the right Program subclass

    std::unique_ptr<Program> best_program;
    emp::vector<double> best_fitness_history;
    emp::vector<double> avg_fitness_history;
    emp::vector<double> median_fitness_history;

    // Effect histories are one element smaller than fitness histories
    emp::vector<double> quality_gain_history; 
    emp::vector<double> success_rate_history;

    emp::vector<double> semantic_intron_history;
    emp::vector<double> semantic_intron_elim_history;
    emp::vector<double> structural_intron_history;

    // Behaviors collected from the population (changes every generation)
    emp::vector<std::pair<double, double>> pop_behavior_set;
    // Behaviors collected across all generations, all runs
    emp::vector<std::pair<double, double>> all_behaviors; 

    // ---- NOVELTY SEARCH ----
    // Archive of diverse past behaviors (MazeProg only)
    emp::vector<std::unique_ptr<Program>> archive;
    double p_min {7.0};

    // Secondary fitness (objective-based) statistics
    // Used to keep track of both novelty and objective fitness simultaneously
    // Has no effect on selection
    // We expect novelty to lead to lower average (objective) fitness, but possibly a higher best fitness
    std::unique_ptr<Program> best_program2;
    emp::vector<double> second_best_fitness_history;
    emp::vector<double> second_avg_fitness_history;
    emp::vector<double> second_median_fitness_history;
    emp::vector<double> p_min_history; // novelty threshold for archive
    
    std::unique_ptr<Evaluator> second_evaluator;

    // ------------------------

    std::mt19937 rng;

    bool verbose;
    std::ostream & os;

public:
    Estimator(std::unique_ptr<Evaluator> eval, 
        emp::vector<std::unique_ptr<Variator>> vars,
        std::unique_ptr<Selector> sel,
        std::unique_ptr<Program> prot,
        std::unique_ptr<Evaluator> s_eval = nullptr,
        bool verbose=false,
        std::ostream & os=std::cout)
      : operators(GLOBAL_OPERATORS),
        constants(GLOBAL_CONSTANTS),
        evaluator(std::move(eval)), 
        variators(std::move(vars)),
        selector(std::move(sel)),
        prototype(std::move(prot)),
        second_evaluator(std::move(s_eval)),
        rng(SEED),
        verbose(verbose),
        os(os) {}
    
    Program const & GetBestProgram() const { return *best_program; }

    void InitPopulation() {
        for (size_t i {0}; i < pop_size; ++i) {
            std::unique_ptr<Program> prog {prototype->New()};
            population.emplace_back(std::move(prog));
        }
    }

    void EvalPopulation() {
        for (std::unique_ptr<Program> & p : population) {
            p->SetFitness(evaluator->Evaluate(*p));
        }
    }

    // Secondary fitness; has no effect on selection
    void EvalPopulationSecondary() {
        assert(second_evaluator && "No secondary evaluator has been set.");
        for (std::unique_ptr<Program> & p : population) {
            p->SetSecondFitness(second_evaluator->Evaluate(*p));
        }
    }

    // Evaluates BEHAVIOR of each program in the population
    // Appends them to the population behavior set
    // Must be called BEFORE EvalPopulation() (which calculates NOVELTY)
    void UpdatePopulationBehaviorSet() {
        pop_behavior_set.clear();
        
        // MazeNoveltyEvaluator & eval {dynamic_cast<MazeNoveltyEvaluator&>(*evaluator)};
        MazeEvaluator & eval {dynamic_cast<MazeEvaluator&>(*evaluator)};

        for (std::unique_ptr<Program> & p : population) {
            MazeProgram & prog {dynamic_cast<MazeProgram&>(*p)};

            prog.SetBehavior(eval.EvaluateBehavior(prog));
            pop_behavior_set.emplace_back(prog.GetBehavior());
        }

        // for (auto & b : pop_behavior_set) {
        //     std::cout << "DEBUG Behavior: (" << b.first << ", " << b.second << ")\n";
        // }
    }


    // ---- NOVELTY SEARCH FUNCTIONS (WIP) ----

    // To be added to the archive, programs must exceed p_min in terms of fitness 
    // Originally intended for novelty search (novelty is stored as fitness)
    // For now, p_min is static. 
    // Must be called AFTER EvalPopulation()
    size_t UpdateArchive() {
        size_t addition_count {0};
        for (std::unique_ptr<Program> & p : population) {
            if (p->GetFitness() > p_min) { // we're maximizing
                archive.emplace_back(p->Clone());
                ++addition_count;
            }
        }

        return addition_count; // returns number of archive additions to adjust p_min
    }
    // ------------------------------

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

    double AvgSecondFitness() {
        assert(second_evaluator && "No secondary evaluator has been set.");
        double total {0};
        for (std::unique_ptr<Program> const & p : population) {
            total += p->GetSecondFitness();
        }
        return total / population.size();
    }

    double MedianFitness() {
        // emp::vector<std::unique_ptr<Program>> pop_ptr_copy;
        emp::vector<double> fitnesses;
        for (size_t i {0}; i < pop_size; ++i) {
            // pop_ptr_copy.emplace_back(population[i]->Clone());
            fitnesses.emplace_back(population[i]->GetFitness());
        }

        std::sort(fitnesses.begin(), fitnesses.end());

        size_t mid {fitnesses.size() / 2};
        if (fitnesses.size() % 2 == 0) {
            return (fitnesses[mid - 1] + fitnesses[mid]) / 2;
        }
        else {
            return fitnesses[mid];
        }
        return 0;
        
    }

    double MedianSecondFitness() {
        assert(second_evaluator && "No secondary evaluator has been set.");
        emp::vector<double> fitnesses;
        for (size_t i {0}; i < pop_size; ++i) {
            // pop_ptr_copy.emplace_back(population[i]->Clone());
            fitnesses.emplace_back(population[i]->GetSecondFitness());
        }

        std::sort(fitnesses.begin(), fitnesses.end());

        size_t mid {fitnesses.size() / 2};
        if (fitnesses.size() % 2 == 0) {
            return (fitnesses[mid - 1] + fitnesses[mid]) / 2;
        }
        else {
            return fitnesses[mid];
        }
        return 0;
    }


    void Reset() {
        population.clear();
        best_program.reset();
        best_program2.reset();

        best_fitness_history.clear();
        avg_fitness_history.clear();
        median_fitness_history.clear();
        
        second_best_fitness_history.clear();
        second_avg_fitness_history.clear();
        second_median_fitness_history.clear();

        all_behaviors.clear();
        archive.clear();
        p_min_history.clear();

        // quality_gain_history.clear();
        // success_rate_history.clear();

        // semantic_intron_history.clear();
        // semantic_intron_elim_history.clear();
        // structural_intron_history.clear();
    }

    void Evolve() { 
        if (verbose) { PrintRunParam(os); }

        if (second_evaluator) p_min_history.emplace_back(p_min);

        InitPopulation(); 

        // // ---- NOVELTY SEARCH ----
        // MazeNoveltyEvaluator & novelty_eval {dynamic_cast<MazeNoveltyEvaluator&>(*evaluator)};
        // MazeEvaluator & fitness_eval {dynamic_cast<MazeEvaluator&>(*second_evaluator)};
    
        // // Make sure training cases are consistent across both evaluators
        // fitness_eval.SetTrainingMazes(novelty_eval.GetTrainingMazes());
        // // ------------------------

        UpdatePopulationBehaviorSet();
        all_behaviors.insert(all_behaviors.end(), pop_behavior_set.begin(), pop_behavior_set.end());

        // // ---- NOVELTY SEARCH ----
        // novelty_eval.SetOtherBehaviors(pop_behavior_set); // necessary before novelty evaluation
        // // ------------------------

        EvalPopulation(); // evaluate their fitness (or novelty in the case of NS)

        // // ---- NOVELTY SEARCH ----
        // if (second_evaluator) EvalPopulationSecondary();
        // UpdateArchive(); // archive updates always happen after EvalPopulation()
        // // ------------------------

        auto best_it = std::max_element(population.begin(), population.end(),
            [](std::unique_ptr<Program> const & a, std::unique_ptr<Program> const & b) {
                return a->GetFitness() < b->GetFitness();
            });
        best_program = (*best_it)->Clone();
        best_fitness_history.emplace_back(best_program->GetFitness());
        avg_fitness_history.emplace_back(AvgFitness());
        median_fitness_history.emplace_back(MedianFitness());


        // Secondary fitness metrics
        if (second_evaluator) {
            auto best_it2 = std::max_element(population.begin(), population.end(),
                [](std::unique_ptr<Program> const & a, std::unique_ptr<Program> const & b) {
                    return a->GetSecondFitness() < b->GetSecondFitness();
                });
            best_program2 = (*best_it2)->Clone();
            second_best_fitness_history.emplace_back(best_program2->GetSecondFitness());
            second_avg_fitness_history.emplace_back(AvgSecondFitness());
            second_median_fitness_history.emplace_back(MedianSecondFitness());
        }


        // semantic_intron_history.emplace_back(AvgSemanticIntronProp());
        // semantic_intron_elim_history.emplace_back(AvgSemanticIntronProp_Elimination());
        // structural_intron_history.emplace_back(AvgStructuralIntronProp());
        
        // Begin evolutionary loop

        size_t no_addition_counter {0};
        for (size_t gen {0}; gen < gens; ++gen) {
            if (verbose) { 
                PrintGenSummary(gen, best_fitness_history[gen], avg_fitness_history[gen], median_fitness_history[gen], os); 
            }

            if (verbose && gen % 10 == 0) {
                os << "First 5 fitnesses: ";
                for (int i {0}; i < 5; ++i) os << population[i]->GetFitness() << " ";
                os << "\n";
            }

            emp::vector<std::unique_ptr<Program>> new_pop;
            // int success_count {0}; // For measuring success rate

            // ---- ELITISM ----
            if (elitism_count > 0) {
                // Sort the population based on fitness (highest first)
                emp::vector<std::unique_ptr<Program>> pop_copy;
                for (auto & p : population) pop_copy.emplace_back(p->Clone());
            
                std::sort(pop_copy.begin(), pop_copy.end(),
                    [](std::unique_ptr<Program> & a, std::unique_ptr<Program> & b) {
                        return a->GetFitness() > b->GetFitness();
                    }
                );
            
                for (size_t i {0}; i < elitism_count && i < pop_copy.size(); ++i) {
                    new_pop.emplace_back(std::move(pop_copy[i]));
                }
            }
            // -----------------
            

            // Produce children - by default, we replace the entire population
            while (new_pop.size() < pop_size) {
                Program const & parent1 {selector->Select(population)};
                Program const & parent2 {selector->Select(population)};

                std::unique_ptr<Program> child {parent1.Clone()}; // Default: copy parent1

                // Not sure if this is a good way
                // Be careful with ordering of variators in set
                // If no binary variator exists, we just mutate child, which is a copy of parent1

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


                new_pop.emplace_back(std::move(child));
            }

            // Update population
            population = std::move(new_pop);
            UpdatePopulationBehaviorSet();
            all_behaviors.insert(all_behaviors.end(), pop_behavior_set.begin(), pop_behavior_set.end());          
            EvalPopulation(); 
            
            // // ---- NOVELTY SEARCH ----
            // EvalPopulationSecondary();
            // size_t addition_count {UpdateArchive()};

            // if (verbose) std::cout << "DEBUG Archive Size: " << archive.size() << std::endl;

            // // Concatenate archive and population behavior set
            // emp::vector<std::pair<double, double>> combined_behaviors {pop_behavior_set};
            // for (std::unique_ptr<Program> & prog_ptr : archive) {
            //     MazeProgram& prog = dynamic_cast<MazeProgram&>(*prog_ptr);
            //     combined_behaviors.emplace_back(prog.GetBehavior());
            // }
            // novelty_eval.SetOtherBehaviors(combined_behaviors);

            // // Adjust p_min dynamically
            // // If no new additions over 25 generations, decrease p_min
            // // If more than 4 additions within a single generation, increase p_min
            // if (addition_count > 4) p_min *= 1.05;

            // if (addition_count == 0) ++no_addition_counter;
            // else no_addition_counter = 0; // reset

            // if (no_addition_counter >= 250) {
            //     p_min *= 0.95;
            //     no_addition_counter = 0;
            // }

            // p_min_history.emplace_back(p_min);

            // if (verbose) os << "P_min: " << p_min << std::endl;
            // // ------------------------

            auto best_it = std::max_element(population.begin(), population.end(),
            [](std::unique_ptr<Program> const & a, std::unique_ptr<Program> const & b) {
                return a->GetFitness() < b->GetFitness();
            });
            best_program = (*best_it)->Clone();
            best_fitness_history.emplace_back(best_program->GetFitness());
            avg_fitness_history.emplace_back(AvgFitness());
            median_fitness_history.emplace_back(MedianFitness());


            // Secondary fitness metrics
            if (second_evaluator) {
                auto best_it2 = std::max_element(population.begin(), population.end(),
                [](std::unique_ptr<Program> const & a, std::unique_ptr<Program> const & b) {
                    return a->GetSecondFitness() < b->GetSecondFitness();
                });
                best_program2 = (*best_it2)->Clone();
                second_best_fitness_history.emplace_back(best_program2->GetSecondFitness());
                second_avg_fitness_history.emplace_back(AvgSecondFitness());
                second_median_fitness_history.emplace_back(MedianSecondFitness());
            }

            // double prev_avg_fitness {avg_fitness_history.back()};
            // double curr_avg_fitness {AvgFitness()};
            // Inverse, makes more sense for quality gain to increase
            // double quality_gain {-(curr_avg_fitness - prev_avg_fitness)}; 
            // avg_fitness_history.emplace_back(curr_avg_fitness);
            // quality_gain_history.emplace_back(quality_gain);

            // success_rate_history.emplace_back(static_cast<double>(success_count) / static_cast<double>(pop_size));

            // median_fitness_history.emplace_back(MedianFitness());

            // semantic_intron_history.emplace_back(AvgSemanticIntronProp());
            // semantic_intron_elim_history.emplace_back(AvgSemanticIntronProp_Elimination());
            // structural_intron_history.emplace_back(AvgStructuralIntronProp());
        }
        
        if (verbose) {
            os << "\nEvolution complete (^_^)!\nOverall Best Fitness: " << best_program->GetFitness() << "\n";
        }

        // // ---- COMMENT IF MULTI-RUN ----
        // ExportFitnessHistory(); // Comment out if doing multi-runs
        // if (second_evaluator) ExportSecondHistory();

        // // Behavior of program with best "fitness"
        // // In the case of novelty, program with highest novelty score
        // // Keep in mind novelty score is relative, 
        // // and doesn't always equate to program with highest objective fitness
        // // This is more suitable for objective-based search
        // ExportBestProgram(); 

        // // -------------------------------

        // // ---- NOVELTY SEARCH ----
        // ExportArchiveBehaviors(); 
        // // ------------------------
    }

    void MultiRunEvolve(int run_count=10, std::ostream & os=std::cout) {
        verbose = false; // just in case

        for (int i {0}; i < run_count; ++i) {
            rng.seed(i);
            Reset();
            Evolve();
            ExportFitnessHistory("fitness_run_" + std::to_string(i) + ".csv");
            ExportAllBehaviors("all_behaviors" + std::to_string(i) + ".csv");
            // ExportEffectHistory("effect_run_" + std::to_string(i) + ".csv");
            // ExportIntronHistory("intron_run_" + std::to_string(i) + ".csv");

            // ---- FOR TEST SET EVALUATION ----
            std::ofstream ofs("best_program_" + std::to_string(i) + ".txt");
            if (ofs.is_open()) {
                // ofs << best_program->GetFitness() << "\n";
                ofs << *best_program;
            }

            // // ---- FOR OBJECTIVE SEARCH ONLY (TO ISOLATE SUFFICIENTLY DIFFICULT MAZES) ----
            // if (!second_evaluator) { // objective search only
            //     MazeEvaluator & eval = dynamic_cast<MazeEvaluator&>(*evaluator);
            //     std::vector<double> maze_dists = eval.EvaluatePerMaze(*best_program);
                
            //     // store best program's performance on individual mazes
            //     std::ofstream dist_ofs("maze_dists_run_" + std::to_string(i) + ".csv"); 
            //     std::string maze_dir = "hard_mazes_run_" + std::to_string(i);
            //     std::filesystem::create_directory(maze_dir);
            
            //     std::vector<MazeEnvironment> const & mazes = eval.GetInputMazes();
            
            //     for (size_t j = 0; j < maze_dists.size(); ++j) {
            //         dist_ofs << j << "," << maze_dists[j] << "\n";
            
            //         if (maze_dists[j] > 20) { // agent does poorly on maze
            //             std::string filename = maze_dir + "/maze_" + std::to_string(j) + ".txt";
            //             mazes[j].SaveMaze(filename);
            //         }
            //     }
            // }


            if (second_evaluator) {
                ExportSecondHistory("second_fitness_run_" + std::to_string(i) + ".csv");
                ExportArchiveBehaviors("behavior_archive_" + std::to_string(i) + ".txt");
                ExportPMinHistory("p_min_history_" + std::to_string(i) + ".csv");

                std::ofstream ofs("best_program2_" + std::to_string(i) + ".txt");
                if (ofs.is_open()) {
                    // ofs << best_program2->GetFitness() << "\n";
                    ofs << *best_program2;
                }
            }

            if (second_evaluator) {
                os << "Best novelty: " << best_program->GetFitness() << "\n";
                os << "Best objective: " << best_program2->GetSecondFitness() << "\n";
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

    // Secondary objective-based fitness
    void ExportSecondHistory(std::string const & filename="second_fitness_history.csv") const {
        assert(second_evaluator && "No secondary evaluator has been set.");
        assert(second_best_fitness_history.size() == second_avg_fitness_history.size());
        assert(second_avg_fitness_history.size() == second_median_fitness_history.size());

        std::ofstream ofs(filename);
        if (ofs.is_open()) {
            ofs << "Generation,BestFitness,MedianFitness,AvgFitness\n";
            ofs << std::fixed << std::setprecision(6);
            for (size_t i {0}; i < second_best_fitness_history.size(); ++i) {
                ofs << i << "," 
                    << second_best_fitness_history[i] << ","
                    << second_median_fitness_history[i] << ","
                    << second_avg_fitness_history[i] << "\n";
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

    void ExportBestProgram(std::string const & filename="best_program.txt") const {
        MazeProgram & best_prog {dynamic_cast<MazeProgram&>(*best_program)};
        std::ofstream ofs(filename);
        if (ofs.is_open()) {
            // Maze Program only :') 
            ofs << best_prog
                << "\nFinal Row: " << best_prog.GetBehaviorR()
                << "\nFinal Col: " << best_prog.GetBehaviorC();

            if (second_evaluator) { 
                ofs << "\nFitness (Distance To Goal): " << best_prog.GetSecondFitness()
                << "\nNovelty (Local Behavioral Distance): " << best_prog.GetFitness() << "\n";
            }
            else {
                ofs << "\nFitness (Distance To Goal): " << best_prog.GetFitness() << "\n";
            }    
        }
    }

    void ExportAllBehaviors(std::string const & filename) const {
        std::ofstream ofs(filename);
        if (ofs.is_open()) {
            ofs << "Row,Col\n";
            for (size_t i {0}; i < all_behaviors.size(); ++i) {
                ofs << all_behaviors[i].first << "," << all_behaviors[i].second << "\n";
            }
        }

    }

    // // ---- NOVELTY SEARCH ----
    void ExportArchiveBehaviors(std::string const & filename="archive_behaviors.txt") const {
        std::ofstream ofs(filename);
        if (ofs.is_open()) {
            for (size_t i {0}; i < archive.size(); ++i) {
                MazeProgram & prog {dynamic_cast<MazeProgram&>(*archive[i])};
                ofs << "Program " << i << "\n" << prog
                << "\nFinal Row: " << prog.GetBehaviorR()
                << "\nFinal Col: " << prog.GetBehaviorC()
                << "\nDistance to goal: " << prog.GetSecondFitness()
                << "\nNovelty: " << prog.GetFitness() << "\n";
            }
        }
    }

    void ExportPMinHistory(std::string const & filename) const {
        std::ofstream ofs(filename);
        if (ofs.is_open()) {
            ofs << "Generation,P_min\n";
            ofs << std::fixed << std::setprecision(6);
            for (size_t i {0}; i < p_min_history.size(); ++i) {
                ofs << i << "," << p_min_history[i] << "\n";
            }
        }
    }
    // ------------------------
};

#endif