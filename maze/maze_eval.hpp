#ifndef MAZE_EVAL_HPP
#define MAZE_EVAL_HPP

#include <vector>
#include <random>
#include <cassert>
#include <iomanip>

#include "emp/base/vector.hpp"

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
            temp.MakeMazeMoreOpen();
            train_mazes.emplace_back(std::move(temp));

            MazeEnvironment temp2(r, c, {1,1}, i);
            temp2.GenerateMazeDFS();
            temp2.MakeMazeMoreOpen();
            train_mazes.emplace_back(std::move(temp2));
        }

        if (maze_count % 2 != 0) {
            MazeEnvironment temp(r, c, {1,1}, SEED);
            temp.GenerateMazeDFS();
            temp.MakeMazeMoreOpen();
            train_mazes.emplace_back(std::move(temp));
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

    // // Simulate program on each maze in the training set
    // void Simulate(MazeProgram & prog) const {   
    //     for (MazeEnvironment & maze : train_mazes) {
    //         SimulateSingleMaze(prog, maze);
    //         // Between mazes/training cases, reset program registers
    //         prog.ResetRegisters();
    //     }
    // }

    // // Simulate program on each maze ine the training set
    // // Evaluate program's behavior (final position, averaged over all training cases)
    //  std::pair<double, double> EvaluateBehavior(MazeProgram & prog) const {
    //     std::pair<double, double> avg_final_pos(0, 0);

    //     for (MazeEnvironment & maze : train_mazes) {

    //         SimulateSingleMaze(prog, maze);

    //         // Get final position of robot in maze
    //         avg_final_pos.first += maze.GetRobotPosition().first; // row
    //         avg_final_pos.second += maze.GetRobotPosition().second; // col

    //         // Between mazes/training cases, reset program registers
    //         prog.ResetRegisters();
    //     }

    //     avg_final_pos.first /= maze_count;
    //     avg_final_pos.second /= maze_count;

    //     // prog.SetBehavior(avg_final_pos); // just... in case??

    //     return avg_final_pos;
    // }


    // // Simulates a single program on all training mazes,
    // // records the final distance from goal in each simulation
    // // then average them to get the program's fitness value. 
    // // The smaller the value, the better (before invert)
    // double Evaluate(Program & p) const override {
    //     MazeProgram & prog = dynamic_cast<MazeProgram&>(p);
    //     // Simulate and evaluate p's behavior
    //     EvaluateBehavior(prog);

    //     // Evaluate p's fitness
    //     double avg_dist {0};
    //     for (MazeEnvironment & maze : train_mazes) {
    //         avg_dist += maze.GetDistToGoal();
    //         maze.ResetRobotPosition(); // reset for the next program 
    //     }
    //     avg_dist /= maze_count;

    //     // p.SetFitness(-avg_dist); // just in case

    //     return -avg_dist; // we're maximizing, so invert
    // }


    // Evaluate program's behavior (final position, averaged over all training cases)
    std::pair<double, double> EvaluateBehavior(MazeProgram & prog) const {
        std::pair<double, double> avg_final_pos(0, 0);

        for (MazeEnvironment & maze : train_mazes) {
            maze.ResetRobotPosition();

            SimulateSingleMaze(prog, maze);

            // Get final position of robot in maze
            avg_final_pos.first += maze.GetRobotPosition().first; // row
            avg_final_pos.second += maze.GetRobotPosition().second; // col

            // Between mazes/training cases, reset program registers
            prog.ResetRegisters();
            maze.ResetRobotPosition();
        }

        avg_final_pos.first /= maze_count;
        avg_final_pos.second /= maze_count;

        // prog.SetBehavior(avg_final_pos); // just... in case??

        return avg_final_pos;
    }



    double Evaluate(Program & p) const override {
        MazeProgram & prog {dynamic_cast<MazeProgram&>(p)};

        double avg_dist {0};
        for (MazeEnvironment & maze : train_mazes) {
            maze.ResetRobotPosition();  
            SimulateSingleMaze(prog, maze);
            avg_dist += maze.GetDistToGoal(); 
            prog.ResetRegisters();
            maze.ResetRobotPosition();
        }
        avg_dist /= maze_count;
    
        return -avg_dist; // Inverted for maximization
    }

    
    std::vector<MazeEnvironment> const & GetInputMazes() const {
        return train_mazes;
    }

    void SetTrainingMazes(std::vector<MazeEnvironment> const & mazes) {
        train_mazes = mazes;
    }

    void SaveTrainingMazes() const {

    }

    void LoadTrainingMazes() {

    }

    std::vector<double> GetInputSet() const override {
        throw std::logic_error("MazeEvaluator::GetInputSet() is not supported.");
    }


};

#endif