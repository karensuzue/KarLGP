#include "maze_global.hpp"

#include <iostream>
#include <fstream>

bool AreMazesEqual(std::vector<std::vector<bool>> const &, std::vector<std::vector<bool>> const &);
bool MazeAlreadyExists(std::vector<MazeEnvironment> const &, MazeEnvironment const &);

int main() {

    std::vector<std::unique_ptr<Variator>> variators;
    variators.emplace_back(std::make_unique<SimpleCrossover>(XOVER_RATE));
    variators.emplace_back(std::make_unique<SimpleMutate>(MUT_RATE));
    std::unique_ptr<Selector> selector {std::make_unique<TournamentSelect>()};
    std::unique_ptr<Program> prototype {std::make_unique<MazeProgram>()};
    
    // // ---- OBJECTIVE SEARCH ----
    // // Up to 11 DFS-generated mazes (43x43) are used in the training set, filtered for misdirection/deception.
    // // DFS-generated mazes feature strong misdirection - many narrow paths lead away from the goal.
    // // For runs that become stuck in local optima, the corresponding maze set is isolated for further runs.
    // std::unique_ptr<Evaluator> evaluator {std::make_unique<MazeEvaluator>(1000, 11, 21, 21)};
    // Estimator est(std::move(evaluator), std::move(variators), std::move(selector), std::move(prototype), nullptr, true);
    // est.MultiRunEvolve();


    // // ---- NOVELTY SEARCH ----
    // // MazeNoveltyEvaluator is the primary driver of evolution, MazeEvaluator is secondary and has no effect
    // // Use maze set from 
    // std::unique_ptr<Evaluator> evaluator {std::make_unique<MazeEvaluator>(1000, 11, 21, 21)};
    // std::unique_ptr<Evaluator> evaluator_novel {std::make_unique<MazeNoveltyEvaluator>(1000, 11, 21, 21, 15)};
    // // std::unique_ptr<Evaluator> evaluator_surprise {std::make_unique<MazeSurpriseEvaluator>(100, 11, 21, 21)};    
    // Estimator est(std::move(evaluator_novel), std::move(variators), std::move(selector), std::move(prototype), std::move(evaluator), true);
    // est.MultiRunEvolve();


    // // ---- OBJECTIVE SEARCH 2 ----
    // // We update the training set to include hard mazes isolated from the first objective search run.
    // // Check for and remove duplicates
    // std::vector<MazeEnvironment> hard_mazes;
    // std::string dirname {"data/finalproj/objective"};
    // for (size_t run {0}; run < 10; ++run) { // 10 runs
    //     for (size_t maze_i {0};; ++maze_i) {
    //         std::string filename = dirname + "/hard_mazes_run_" + std::to_string(run) +
    //                                 "/maze_" + std::to_string(maze_i) + ".txt";
    //         std::ifstream ifs(filename);
    //         if (!ifs.is_open()) break; // stop when no more files

    //         MazeEnvironment maze;
    //         maze.LoadMaze(filename);
    //         if (!MazeAlreadyExists(hard_mazes, maze)) hard_mazes.emplace_back(std::move(maze));
    //     }
    // }
    // assert(!hard_mazes.empty() && "No hard mazes found :/");

    // std::unique_ptr<Evaluator> evaluator {std::make_unique<MazeEvaluator>(1000, 11, 21, 21)};
    // MazeEvaluator & eval {dynamic_cast<MazeEvaluator&>(*evaluator)};
    // eval.SetTrainingMazes(hard_mazes);

    // Estimator est(std::move(evaluator), std::move(variators), std::move(selector), std::move(prototype), nullptr, true);
    // est.MultiRunEvolve();


    // // ---- NOVELTY SEARCH 2 ----
    // // We test Novelty Search on "hard" mazes isolated from the objective-search runs.
    // // Our goal is to see whether Novelty Search can outperform objective search on these mazes under identical conditions.
    // // We expect Novelty Search to get closer to the goal, whereas objective search became trapped in local optima.

    // // Check for and remove duplicates
    // // std::cout << "Current working directory: " << std::filesystem::current_path() << "\n";

    // std::cout << "Checking for and removing duplicates in hard mazes" << std::endl;
    // std::vector<MazeEnvironment> hard_mazes;
    // std::string dirname {"data/finalproj/objective"};
    // for (size_t run {0}; run < 10; ++run) { // 10 runs
    //     for (size_t maze_i {0}; maze_i < 11; ++maze_i) { // 11 mazes total
    //         std::string filename = dirname + "/hard_mazes_run_" + std::to_string(run) +
    //                                 "/maze_" + std::to_string(maze_i) + ".txt";
    //         std::ifstream ifs(filename);
    //         // std::cout << "TRYING TO OPEN: " << filename << std::endl;
    //         if (!ifs.is_open()) continue; // stop when no more files
    //         // std::cout << "OPENED: " << filename << std::endl; 

    //         MazeEnvironment maze;
    //         maze.LoadMaze(filename);
    //         if (!MazeAlreadyExists(hard_mazes, maze)) hard_mazes.emplace_back(std::move(maze));
    //     }
    // }

    // assert(!hard_mazes.empty() && "No hard mazes found :/");

    // std::cout << "Begin evolution" << std::endl;

    // std::unique_ptr<Evaluator> evaluator {std::make_unique<MazeEvaluator>(1000, 11, 21, 21)}; // secondary
    // std::unique_ptr<Evaluator> evaluator_novel {std::make_unique<MazeNoveltyEvaluator>(1000, 11, 21, 21, 15)}; // primary
    // MazeNoveltyEvaluator & eval_novel {dynamic_cast<MazeNoveltyEvaluator&>(*evaluator_novel)};
    // eval_novel.SetTrainingMazes(hard_mazes);

    // Estimator est(std::move(evaluator_novel), std::move(variators), std::move(selector), std::move(prototype), std::move(evaluator), true);
    // est.MultiRunEvolve();



    // ---- TEST SET EVALUATION ----
    // Evaluate best performers from each strategy on unseen mazes

    std::unique_ptr<Evaluator> evaluator {std::make_unique<MazeEvaluator>(1000, 11, 21, 21)};
    MazeEvaluator & eval {dynamic_cast<MazeEvaluator&>(*evaluator)};

    emp::vector<std::unique_ptr<MazeEnvironment>> test_mazes;
    auto maze_dfs {std::make_unique<MazeEnvironment>(21, 21, std::pair<int, int>{1,1}, SEED)};
    maze_dfs->GenerateMazeDFS();
    test_mazes.emplace_back(std::move(maze_dfs));

    auto maze_bin {std::make_unique<MazeEnvironment>(21, 21, std::pair<int, int>{1,1}, SEED)};
    maze_bin->GenerateMazeBinary();
    test_mazes.emplace_back(std::move(maze_bin));

    auto maze_prim {std::make_unique<MazeEnvironment>(21, 21, std::pair<int, int>{1,1}, SEED)};
    maze_prim->GenerateMazePrim(); 
    test_mazes.emplace_back(std::move(maze_prim));
    std::cout << test_mazes.size() << std::endl;

    std::ofstream ofs("test_objective_hard.csv");
    ofs << "Program,DFSDist,BinDist,PrimDist\n";
    std::string filename {"data/finalproj/objective-hard/best_program_"};
    emp::vector<double> dfs_distance; // 'distance' as in distance from goal
    emp::vector<double> bin_distance;
    emp::vector<double> prim_distance;

    for (int i {0}; i < 10; ++i) { // Iterate through best program from each run
        MazeProgram prog;

        prog.LoadMazeProgram(filename + std::to_string(i) + ".txt");
        std::cout << prog << std::endl;

        ofs << i << ",";

        MazeEnvironment maze_dfs {*test_mazes[0]}; 
        maze_dfs.ResetRobotPosition();
        eval.SimulateSingleMaze(prog, maze_dfs);
        double dist {maze_dfs.GetDistToGoal()};
        dfs_distance.push_back(dist);
        ofs << dist << ",";

        MazeEnvironment maze_bin {*test_mazes[1]}; 
        maze_bin.ResetRobotPosition();
        eval.SimulateSingleMaze(prog, maze_bin);
        bin_distance.emplace_back(maze_bin.GetDistToGoal());
        ofs << bin_distance[i] << ",";

        MazeEnvironment maze_prim {*test_mazes[2]}; 
        maze_prim.ResetRobotPosition();
        eval.SimulateSingleMaze(prog, maze_prim);
        prim_distance.emplace_back(maze_prim.GetDistToGoal());
        ofs << prim_distance[i] << "\n";

        std::cout << "Program " << i << ": DFS=" << dfs_distance[i]
          << " Bin=" << bin_distance[i]
          << " Prim=" << prim_distance[i] << "\n";
    }
    
    assert(dfs_distance.size() == bin_distance.size());
    assert(bin_distance.size() == prim_distance.size());

}

bool AreMazesEqual(std::vector<std::vector<bool>> const & maze1, std::vector<std::vector<bool>> const & maze2) {
    if (maze1.size() != maze2.size()) return false;
    for (size_t i {0}; i < maze1.size(); ++i) {
        if (maze1[i].size() != maze2[i].size()) return false; // check row
        for (size_t j {0}; j < maze1[i].size(); ++j) { // check col
            if (maze1[i][j] != maze2[i][j]) return false;
        }
    }
    return true;
}


bool MazeAlreadyExists(std::vector<MazeEnvironment> const & vec, MazeEnvironment const & maze) {
    for (MazeEnvironment const & m : vec) {
        if (AreMazesEqual(m.GetGrid(), maze.GetGrid()) &&
            m.GetStartPosition() == maze.GetStartPosition() &&
            m.GetGoalPosition() == maze.GetGoalPosition()) return true;
    }
    return false;
}