#include "maze_global.hpp"

#include <iostream>
#include <fstream>

int main() {
    MazeEnvironment maze(7, 7, {1,1}); 
    // maze.GenerateMazeDFS();
    // std::cout << maze;

    maze.GenerateMazeBinary();
    std::cout << maze;

    std::ofstream ofs("maze.txt");
    if (ofs.is_open()) { ofs << maze; }

    maze.Step(3);
    maze.Step(3);
    maze.Step(1);
    maze.Step(1);
    std::cout << maze;

    maze.UpdateSensors();
    std::vector<double> sensors = maze.GetSensors();

    for (double val : sensors) {
        std::cout << val << ", ";
    }
    std::cout << std::endl;

    std::cout << maze.GetDistToGoal() << std::endl;
    


}