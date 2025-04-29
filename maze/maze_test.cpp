#include "maze_global.hpp"

#include <iostream>
#include <fstream>

int main() {
    // MazeEnvironment maze(21, 21, {1,1}, SEED);
    // // maze.GenerateMazeBinary();
    // maze.GenerateMazeDFS();
    // // maze.MakeMazeMoreOpen();
    // std::cout << maze;

    std::unique_ptr<Evaluator> evaluator {std::make_unique<MazeEvaluator>(1000, 11, 21, 21)};
    std::unique_ptr<Evaluator> evaluator_novel {std::make_unique<MazeNoveltyEvaluator>(1000, 11, 21, 21, 15)};
    // // // std::unique_ptr<Evaluator> evaluator_surprise {std::make_unique<MazeSurpriseEvaluator>(100, 11, 21, 21)};
    
    std::vector<std::unique_ptr<Variator>> variators;
    variators.emplace_back(std::make_unique<SimpleCrossover>(XOVER_RATE));
    variators.emplace_back(std::make_unique<SimpleMutate>(MUT_RATE));

    std::unique_ptr<Selector> selector {std::make_unique<TournamentSelect>()};

    std::unique_ptr<Program> prototype {std::make_unique<MazeProgram>()};

    // Estimator est(std::move(evaluator), std::move(variators), std::move(selector), std::move(prototype), nullptr, true);
    Estimator est(std::move(evaluator_novel), std::move(variators), std::move(selector), std::move(prototype), std::move(evaluator), true);
    est.Evolve();
    // std::cout << est.GetBestProgram();
    // est.MultiRunEvolve();


}