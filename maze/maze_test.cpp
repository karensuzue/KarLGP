#include "maze_global.hpp"

#include <iostream>
#include <fstream>

int main() {
    MazeEnvironment maze(9, 9, {1,1}); 
    // maze.GenerateMazeDFS();
    // std::cout << maze;

    maze.GenerateMazeBinary();
    std::cout << maze;

    std::ofstream ofs("maze.txt");
    if (ofs.is_open()) { ofs << maze; }
    


}