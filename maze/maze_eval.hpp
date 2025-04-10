#ifndef MAZE_EVAL_HPP
#define MAZE_EVALHPP

class MazeEvaluator : public Evaluator {
private:
    int max_steps;
    std::vector<MazeEnvironment> test_mazes;

public:
    Simulate() {
        // For a single maze:
        // simulate robot till MAX_STEPS
        // repeatedly:
            // execute program
            // Step() 
            // get new inputs from sensor
            // Put inputs in program

    }

    Evaluate() {
        // simulate a single program on all test mazes
        // rec0rd final distance from goal in each simulation
        // average them to get the program's fitness
    }

};

#endif