#ifndef MAZE_ENV_HPP
#define MAZE_ENV_HPP

#include <utility>
#include <vector>
#include <stack>
#include <random>

class MazeEnvironment {
private:
    std::vector<std::vector<bool>> grid; // 0 = open space, 1 = wall
    int rows, cols;
    std::pair<int, int> start; // start coordinate
    std::pair<int, int> goal; // goal coordinate
    std::pair<int, int> position; // robot position

    std::mt19937 rng;

public:
    // Start at {1, 1} to leave room for edge walls
    // Make sure rows and cols are odd
    MazeEnvironment(int r=21, int c=21, std::pair<int,int> start={1,1}) 
    : rows(r), cols(c), start(start), position(start), rng(SEED)
    {
        // GenerateMazeDFS();
    }

    void ImportMaze() {
        // Import maze from text file
        
    }

    void GenerateMazeDFS() {
        // This guarantees a perfect maze (no loops, one path between any 2 points) by building a spanning tree
        // This method back tracks every time a dead end is encountered
        // Root = start cell

        // Assume a wall exists between 2 neighbors
        // We connect neighbors by "knocking down" the wall
        std::vector<std::pair<int,int>> directions = {
            {0, -2}, // up
            {0, 2}, // down
            {-2, 0}, // left
            {2, 0} // right
        };

        // Start by initializing grid with walls
        grid = std::vector<std::vector<bool>> (rows, std::vector<bool> (cols, true));

        // Mark starting cell as open
        grid[start.first][start.second] = false;

        std::stack<std::pair<int,int>> stack;
        stack.push(start);

        // Start DFS traversal
        while (!stack.empty()) {
            std::pair<int,int> current_coord {stack.top()};
            std::vector<std::pair<int,int>> neighbor_coords;

            // Look for unvisited neighbors
            for (auto const & [dx, dy] : directions) {
                int neighbor_x {current_coord.first + dx};
                int neighbor_y {current_coord.second + dy};

                // If neighbor is still a wall and is not past the edges of the maze
                if (neighbor_x > 0 && neighbor_x < rows - 1 &&
                    neighbor_y > 0 && neighbor_y < cols - 1 &&
                    grid[neighbor_x][neighbor_y]) {
                        neighbor_coords.push_back({neighbor_x, neighbor_y});
                    }
            }

            // Choose a random neighbor
            // Carve out a path connecting 'current_coord' and the neighbor
            // Neighbor is pushed into stack, becomes top cell in DFS path
            if (!neighbor_coords.empty()) {
                std::uniform_int_distribution<size_t> dist(0, neighbor_coords.size() - 1);
                std::pair<int,int> chosen_neighbor {neighbor_coords[dist(rng)]};

                int wall_x {(current_coord.first + chosen_neighbor.first) / 2};
                int wall_y {(current_coord.second + chosen_neighbor.second) / 2};

                grid[chosen_neighbor.first][chosen_neighbor.second] = false;
                grid[wall_x][wall_y] = false;

                stack.push(chosen_neighbor);
            }
            // Backtrack by popping current cell from stack
            else {
                stack.pop();
            }
        }

        goal = {rows - 2, 1}; // Generally more deceptive if start aligns with goal column-wise
    }

    void GenerateMazeBinary() {
        // Carve out paths in a diagonal direction, creates a binary tree
        // Also guarantees a perfect maze
        // At each cell, algorithm always move down or right

        // std::vector<std::pair<int,int>> directions = {
        //     {0, -2}, // up
        //     {0, 2}, // down
        //     {-2, 0}, // left
        //     {2, 0} // right
        // };

        // Start by initializing grid with walls
        grid = std::vector<std::vector<bool>> (rows, std::vector<bool> (cols, true));

        // Mark starting cell as open
        // grid[start.first][start.second] = false;

        for (int i {1}; i < rows - 1; i += 2) {
            for (int j {1}; j < cols - 1; j += 2) {
                grid[i][j] = false;
                std::vector<std::pair<int,int>> neighbor_coords;

                // Check for south neighbor
                if (i + 2 < rows - 1) neighbor_coords.push_back({i + 2, j});
                // Check for east neighbor
                if (j + 2 < cols - 1) neighbor_coords.push_back({i, j + 2});

                if (!neighbor_coords.empty()) {
                    // Select a random neighbor
                    std::uniform_int_distribution<> dist(0, neighbor_coords.size() - 1);
                    std::pair<int, int> chosen_neighbor {neighbor_coords[dist(rng)]};

                    // Carve path
                    int wall_x {(i + chosen_neighbor.first) / 2};
                    int wall_y {(j + chosen_neighbor.second) / 2};
                    grid[chosen_neighbor.first][chosen_neighbor.second] = false;
                    grid[wall_x][wall_y] = false;
                }
            }
        }
        goal = {rows - 2, 1};
    }


    // void Reset() {
    //     position = start;
    // }

    // bool IsWall(std::pair<int, int> coord) const {
    //     return grid[coord.first][coord.second] == 1;
    // }

    // bool CanMove(std::pair<int, int> coord) const {
    //     int x {coord.first}, y {coord.second};
    //     return !IsWall(coord) &&
    //            x >= 0 && x < cols &&
    //            y >= 0 && y < rows;
    // }

    // void Step(int action) {
    //     // robot action
    //     // 0 = forward, 1 = down, 2 = left, 3 = right 
    // }

    // std::vector<double> GetSensors() {
    //     // front, left, right walls
    //     // goal angle
    // } 

    // double GetDistToGoal() const {
    //     // for eval module
    // }

    // bool ReachedGoal() const {
    //     // yuh
    // }

    void PrintMaze(std::ostream & os) const {
        for (int i {0}; i < rows; ++i) {
            for (int j {0}; j < cols; ++j) {
                if (std::make_pair(i,j) == position) os << "R"; // robot current position
                else if (std::make_pair(i,j) == start) os << "S";
                else if (std::make_pair(i,j) == goal) os << "G";
                // true = wall, false = path
                else os << (grid[i][j] ? '#' : ' ');
            }
            os << "\n";
        }

    }

    friend std::ostream & operator<<(std::ostream & os, MazeEnvironment const & maze) {
        maze.PrintMaze(os);
        return os;
    }
};

#endif