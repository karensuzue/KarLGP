#include "maze_global.hpp"

#include <iostream>
#include <fstream>

int main() {
    // Create new maze, generate paths, save
    // MazeEnvironment maze(7, 7, {1,1}, SEED); 
    // maze.GenerateMazeBinary();
    // std::ofstream ofs("maze.txt");
    // if (ofs.is_open()) { ofs << maze; }


    // MazeProgram program;
    // std::cout << program;

    // MazeEvaluator evaluator(2000, 11, 7, 7);
    // // eval.SimulateSingleMaze(program, new_maze);

    // double fitness {evaluator.Evaluate(program)};
    // std::cout << "fitness is " << fitness << std::endl;
    // std::cout << "behavior is " << program.GetBehavior() << std::endl;

    // std::unique_ptr<Evaluator> evaluator {std::make_unique<MazeEvaluator>(100, 11, 21, 21)};
    std::unique_ptr<Evaluator> evaluator_novel {std::make_unique<MazeNoveltyEvaluator>(100, 11, 21, 21, 10)};
    // std::unique_ptr<Evaluator> evaluator_surprise {std::make_unique<MazeSurpriseEvaluator>(100, 11, 21, 21)};
    
    std::vector<std::unique_ptr<Variator>> variators;
    variators.push_back(std::make_unique<SimpleCrossover>(XOVER_RATE));
    variators.push_back(std::make_unique<SimpleMutate>(MUT_RATE));

    std::unique_ptr<Selector> selector {std::make_unique<TournamentSelect>()};

    std::unique_ptr<Program> prototype {std::make_unique<MazeProgram>()};

    Estimator est(std::move(evaluator_novel), std::move(variators), std::move(selector), std::move(prototype), true);
    est.Evolve();
    std::cout << est.GetBestProgram();


}