#ifndef MAZE_ENV_HPP
#define MAZE_ENV_HPP

#include <utility>
#include <vector>
#include <stack>
#include <random>
#include <cmath>

class MazeEnvironment {
private:
    std::vector<std::vector<bool>> grid; // 0 = open space, 1 = wall
    int rows, cols;
    std::pair<int, int> start; // start coordinate
    std::pair<int, int> goal; // goal coordinate

    std::pair<int, int> position; // robot position
    std::vector<double> sensors{5}; // robot sensors

    std::mt19937 rng;

    // Robot action indices (default):
    // 0 = up, 1 = down, 2 = left, 3 = right 
    std::vector<std::pair<int, int>> moves;

public:
    // Start at {1, 1} to leave room for edge walls
    // Make sure rows and cols are odd
    MazeEnvironment(int r=21, int c=21, std::pair<int, int> start={1,1}, int seed=SEED) 
    : rows(r), cols(c), start(start), position(start), rng(seed)
    {
        // GenerateMazeDFS();
        // goal = {rows - 2, 1};
        // GenerateGoal();

        // Initialize moves set
        moves = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    }

    void ImportMaze() {
        // Import maze from text file
        
    }

    void GenerateGoal() {
        // Goal should preferably be towards the bottom half
        std::uniform_int_distribution<int> row_dist(rows / 2, rows - 2);
        std::uniform_int_distribution<int> col_dist(1, cols - 2);
        goal = {row_dist(rng), col_dist(rng)};
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
            for (auto const & [dr, dc] : directions) {
                int neighbor_r {current_coord.first + dr};
                int neighbor_c {current_coord.second + dc};

                // If neighbor is still a wall and is not past the edges of the maze
                if (neighbor_r > 0 && neighbor_r < rows - 1 &&
                    neighbor_c > 0 && neighbor_c < cols - 1 &&
                    grid[neighbor_r][neighbor_c]) {
                        neighbor_coords.push_back({neighbor_r, neighbor_c});
                    }
            }

            // Choose a random neighbor
            // Carve out a path connecting 'current_coord' and the neighbor
            // Neighbor is pushed into stack, becomes top cell in DFS path
            if (!neighbor_coords.empty()) {
                std::uniform_int_distribution<size_t> dist(0, neighbor_coords.size() - 1);
                std::pair<int,int> chosen_neighbor {neighbor_coords[dist(rng)]};

                int wall_r {(current_coord.first + chosen_neighbor.first) / 2};
                int wall_c {(current_coord.second + chosen_neighbor.second) / 2};

                grid[chosen_neighbor.first][chosen_neighbor.second] = false;
                grid[wall_r][wall_c] = false;

                stack.push(chosen_neighbor);
            }
            // Backtrack by popping current cell from stack
            else {
                stack.pop();
            }
        }

        // goal = {rows - 2, 1}; // Generally more deceptive if start aligns with goal column-wise
        GenerateGoal();
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
                    int wall_r {(i + chosen_neighbor.first) / 2};
                    int wall_c {(j + chosen_neighbor.second) / 2};
                    grid[chosen_neighbor.first][chosen_neighbor.second] = false;
                    grid[wall_r][wall_c] = false;
                }
            }
        }
        // goal = {rows - 2, 1};
        GenerateGoal();
    }

    std::pair<int, int> GetRobotPosition() const {
        return position;
    }

    void ResetRobotPosition() {
        position = start;
    }

    void ResetMaze() {
        grid.clear();
    }

    bool IsWall(std::pair<int, int> coord) const {
        return coord.first >= 0 && coord.first < rows &&
               coord.second >= 0 && coord.second < cols && 
               grid[coord.first][coord.second] == 1;
    }

    bool CanMove(std::pair<int, int> coord) const {
        // Can move if r and c are not out of bounds, and we're not hitting a wall
        int r {coord.first}, c {coord.second};
        return r >= 0 && r < rows &&
               c >= 0 && c < cols &&
               !IsWall(coord);
    }

    void Step(int action) {
        // Robot action indices:
        // 0 = up, 1 = down, 2 = left, 3 = right 
        auto [dr, dc] {moves[action]};
        std::pair<int, int> new_pos = {position.first + dr, position.second + dc}; 
        if (CanMove(new_pos)) position = new_pos;
    }

    // Sensor data is used as input for the program
    // Based on different sensor inputs, the program decides which action to take
    void UpdateSensors() {
        double up {IsWall({position.first - 1, position.second}) ? 1.0 : 0.0};
        double down {IsWall({position.first + 1, position.second}) ? 1.0 : 0.0};
        double left {IsWall({position.first, position.second - 1}) ? 1.0 : 0.0};
        double right {IsWall({position.first, position.second + 1}) ? 1.0 : 0.0};
        double angle {GetGoalAngle()};
        sensors = {up, down, left, right, angle};
    } 

    std::vector<double> GetSensors() const {
        return sensors;
    }

    double GetGoalAngle() const {
        int dx {goal.second - position.second}; // x is cols
        int dy {goal.first - position.first}; // y is rows

        double angle {std::atan2(dy, dx)}; // [-pi, pi]

        return angle / M_PI; // [-1, 1]
    }

    double GetDistToGoal() const {
        // return std::sqrt(std::pow(goal.second - position.second, 2) + std::pow(goal.first - position.first, 2));
        int dx {goal.second - position.second};
        int dy {goal.first - position.first};
        return std::hypot(dx, dy);
    }

    bool ReachedGoal() const {
        return position == goal;
    }

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