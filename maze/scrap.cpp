// This function doesn’t clearly fit into any class, so it’s placed here for the time being
// For a single maze, evolve a small population of controllers over multiple runs.
// If any controller gets sufficiently close to the goal in a run, that run is counted as a success.
// If the number of successful runs exceeds a threshold, the maze is considered too easy.
bool IsTooEasy(MazeEnvironment const & maze_template, MazeEvaluator const & eval, SimpleMutate const & mut, 
                int runs=5, int gens=1000, int pop_size=200) {

    double GOAL_THRESH {-10};
    int MIN_SUCCESSES {3};

    int success_count {0};
    for (int r {0}; r < runs; ++r) {
        // Create a population of random MazePrograms
        emp::vector<MazeProgram> pop;
        for (int i {0}; i < pop_size; ++i) {
            pop.emplace_back();
        }

        for (int g {0}; g < gens; ++g) {
            // ptrs to circumvent errors 
            std::vector<std::pair<double, std::unique_ptr<MazeProgram>>> fit_pop; // what have i done
            for (MazeProgram & p : pop) {
                MazeEnvironment maze {maze_template};  // fresh copy
                double fitness {0};
                maze.ResetRobotPosition();
                eval.SimulateSingleMaze(p, maze);
                fitness = -(maze.GetDistToGoal());
                fit_pop.emplace_back(fitness, std::make_unique<MazeProgram>(p));
            }

            // Sort and check best
            std::sort(
                fit_pop.begin(),
                fit_pop.end(),
                [](auto const & a, auto const & b) {
                    return a.first > b.first;
                }
            );

            if (fit_pop[0].first >= GOAL_THRESH) {
                ++success_count;
                break; // end run
            }

            // Selection + mutation
            pop.clear();
            for (int i {0}; i < pop_size; ++i) {
                MazeProgram offspring = *fit_pop[i % 10].second; // dereference unique_ptr
                mut.Apply(offspring);
                pop.push_back(std::move(offspring));
            }
        }
    }
    return success_count >= MIN_SUCCESSES; // if success count exceeds minimum, it's too easy
}

    // ---- PRELIMINARY DECEPTION TESTING ----
    // Get training set
    std::cout << "DEBUG Begin preliminary testing" << std::endl;
    MazeEvaluator & eval {dynamic_cast<MazeEvaluator&>(*evaluator)};
    std::vector<MazeEnvironment> mazes {eval.GetInputMazes()};
    std::vector<MazeEnvironment> mazes_deceptive;
    SimpleMutate mut(MUT_RATE);

    std::cout << "DEBUG IsTooEasy called" << std::endl;
    for (size_t i {0}; i < mazes.size(); ++i) {
        if (!IsTooEasy(mazes[i], eval, mut)) mazes_deceptive.push_back(mazes[i]);
    }

    std::cout << "DEBUG Test SetTrainingMazes" << std::endl;
    eval.SetTrainingMazes(mazes_deceptive);

    mazes = eval.GetInputMazes();
    assert(mazes.size() == mazes_deceptive.size());

    std::cout << "DEBUG Exporting mazes" << std::endl;
    for (size_t i {0}; i < mazes.size(); ++i) {
        std::ofstream ofs("test_" + std::to_string(i) + ".txt");
        ofs << mazes[i];
    }




     // TEST
    // MazeEvaluator & eval {dynamic_cast<MazeEvaluator&>(*evaluator)};
    // eval.SaveTrainingMazes();

    // std::unique_ptr<Evaluator> evaluator2 {std::make_unique<MazeEvaluator>(1000, 11, 21, 21)};
    // MazeEvaluator & eval2 {dynamic_cast<MazeEvaluator&>(*evaluator2)};
    // eval2.LoadTrainingMazes();

    // emp::vector<MazeEnvironment> mazes {eval.GetInputMazes()};
    // emp::vector<MazeEnvironment> mazes2 {eval.GetInputMazes()};
    // assert(mazes.size() == mazes2.size());
    // for (size_t i {0}; i < mazes.size(); ++i) {
    //     assert(mazes[i].AreMazesEqual(mazes[i].grid, mazes2[i].grid));
    // }