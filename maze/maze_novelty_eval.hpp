// Novelty replaces fitness

#ifndef MAZE_NOVELTY_EVAL_HPP
#define MAZE_NOVELTY_EVAL_HPP

#include <vector>
#include <random>
#include <cassert>
#include <algorithm>

#include <iomanip>

#include "../core/base_eval.hpp"

class MazeNoveltyEvaluator : public Evaluator {
private:
    size_t max_steps;
    size_t maze_count;
    size_t maze_row, maze_col;
    mutable std::vector<MazeEnvironment> train_mazes; // training cases

    std::vector<std::pair<double, double>> other_behaviors;
    size_t k; // number of neighbors for novelty calculation


    std::mt19937 rng;

public:
    MazeNoveltyEvaluator(size_t m_steps, size_t m_count, size_t r, size_t c, size_t k) 
        : max_steps(m_steps), maze_count(m_count), maze_row(r), maze_col(c), k(k) {
        
        // Generate set of training mazes (half binary, half DFS)
        for (size_t i {0}; i < maze_count / 2; ++i) {
            // Seed is taken from indices to vary goal positions
            MazeEnvironment temp(r, c, {1,1}, i);
            temp.GenerateMazeBinary();
            train_mazes.push_back(std::move(temp));

            MazeEnvironment temp2(r, c, {1,1}, i);
            temp2.GenerateMazeDFS();
            train_mazes.push_back(std::move(temp2));
        }

        if (maze_count % 2 != 0) {
            MazeEnvironment temp(r, c, {1,1}, SEED);
            temp.GenerateMazeDFS();
            train_mazes.push_back(std::move(temp));
        }
    }

    // Must be called before Evaluate()
    void SetOtherBehaviors(std::vector<std::pair<double, double>> const & ob) {
        other_behaviors = ob;
    }
    
    // Simulate program on a single maze/training case till MAX_STEPS or till goal reached
    void SimulateSingleMaze(MazeProgram & prog, MazeEnvironment & maze) const {
        for (size_t step {0}; step < max_steps; ++step) {
            maze.UpdateSensors();

            prog.Input(maze.GetSensors());
            prog.ExecuteProgram();

            maze.Step(prog.GetOutputStep());

            // Stop when goal is reached?
            if (maze.GetDistToGoal() == 0) {
                return; 
            }
        }
    }


    double BehaviorDistance(std::pair<double, double> const & a, std::pair<double, double> const & b) const {
        double dx = a.first - b.first;
        double dy = a.second - b.second;
        return std::sqrt(dx * dx + dy * dy);
    }

    // Evaluate program's behavior (final position, averaged over all training cases)
    std::pair<double, double> EvaluateBehavior(MazeProgram & prog) const {
        std::pair<double, double> avg_final_pos(0, 0);

        for (MazeEnvironment & maze : train_mazes) {

            SimulateSingleMaze(prog, maze);

            // Get final position of robot in maze
            avg_final_pos.first += maze.GetRobotPosition().first;
            avg_final_pos.second += maze.GetRobotPosition().second;

            // Between mazes/training cases, reset program registers
            prog.ResetRegisters();
        }

        avg_final_pos.first /= maze_count;
        avg_final_pos.second /= maze_count;

        prog.SetBehavior(avg_final_pos); // just... in case??

        return avg_final_pos;
    }

    // Assumes behaviors have been calculated for the entire population
    // Novelty is the average behavioral distance between an individual and 
        // its k-closest neighbors in the population and archive (concatenated)
    double Evaluate(Program & p) const override {
        assert(!other_behaviors.empty() && "Cannot compute novelty: behavior set is empty.");
        MazeProgram & prog = dynamic_cast<MazeProgram&>(p);
        assert(prog.IsBehaviorEvaluated() && "Program behavior has not been evaluated yet.");

        // Calculate behavioral distances between current program and 
        // others in the population and archive
        std::vector<double> distances;
        for (std::pair<double, double> const & neighbor : other_behaviors) {
            distances.emplace_back(BehaviorDistance(prog.GetBehavior(), neighbor));
        }

        // Sort behavior distances to obtain k-nearest neighbors
        std::sort(distances.begin(), distances.end());
        int novelty {0};
        for (size_t i {0}; i < k; ++i) {
            novelty += distances[i];
        }
        
        // Novelty will be stored as fitness in program
        return novelty / k;  
    }



    std::vector<MazeEnvironment> const & GetInputMazes() const {
        return train_mazes;
    }

    std::vector<double> GetInputSet() const override {
        assert(false && "MazeEvaluator::GetInputSet() is not supported.");
        return std::vector<double>();
    }


};


#endif