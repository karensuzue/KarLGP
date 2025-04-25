#ifndef MAZE_EVAL_HPP
#define MAZE_EVAL_HPP

#include <vector>
#include <random>
#include <cassert>

#include <iomanip>

#include "../core/base_eval.hpp"

class MazeEvaluator : public Evaluator {
private:
    size_t max_steps;
    size_t maze_count;
    size_t maze_row, maze_col;
    mutable std::vector<MazeEnvironment> train_mazes; // training cases
    std::mt19937 rng;

public:
    MazeEvaluator(size_t m_steps, size_t m_count, size_t r, size_t c) 
        : max_steps(m_steps), maze_count(m_count), maze_row(r), maze_col(c) {
        
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

    // Simulate program on each maze in the training set
    void Simulate(MazeProgram & prog) const {   
        for (MazeEnvironment & maze : train_mazes) {
            SimulateSingleMaze(prog, maze);
            // Between mazes/training cases, reset program registers
            prog.ResetRegisters();
        }
    }

    // Simulates a single program on all training mazes,
    // records the final distance from goal in each simulation
    // then average them to get the program's fitness value. 
    // The smaller the value, the better (before invert)
    double Evaluate(Program & p) const override {
        MazeProgram & prog = dynamic_cast<MazeProgram&>(p);
        Simulate(prog);
        double dist_sum {0};
        for (MazeEnvironment & maze : train_mazes) {
            dist_sum += maze.GetDistToGoal();
            maze.ResetRobotPosition(); // reset for the next program 
        }
        return -(dist_sum / maze_count); // we're maximizing, so invert
    }

    std::vector<MazeEnvironment> const & GetInputMazes() const {
        return train_mazes;
    }

    std::vector<double> GetInputSet() const override {
        throw std::logic_error("MazeEvaluator::GetInputSet() is not supported.");
    }


};

#endif