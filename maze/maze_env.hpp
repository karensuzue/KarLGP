#ifndef MAZE_ENV_HPP
#define MAZE_ENV_HPP

#include <utility>
#include <vector>
#include <stack>
#include <random>
#include <cmath>
#include <queue>

#include "emp/base/vector.hpp"

class MazeEnvironment {
private:
    emp::vector<emp::vector<bool>> grid; // 0 = open space, 1 = wall
    int rows, cols;
    std::pair<int, int> start; // start coordinate
    std::pair<int, int> goal; // goal coordinate

    std::pair<int, int> position; // robot position
    emp::vector<double> sensors{5}; // robot sensors

    std::mt19937 rng;

    // Robot action indices (default):
    // 0 = up, 1 = down, 2 = left, 3 = right 
    emp::vector<std::pair<int, int>> moves;

public:
    // Start at {1, 1} to leave room for edge walls
    // Make sure rows and cols are odd
    MazeEnvironment(int cell_count_row=21, int cell_count_col=21, std::pair<int, int> start={1,1}, int seed=SEED) 
    : rows(2 * cell_count_row + 1), cols(2 * cell_count_col + 1), start(start), position(start), rng(seed)
    {
        // GenerateMazeDFS();
        // goal = {rows - 2, 1};
        // GenerateGoal();

        // Initialize moves set
        moves = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    }

    void GenerateGoal() {
        // Vary goal position (but not too much) so that strategy avoids "wall-hugging" and
        // encourages generality 

        // Goal should preferably be towards the bottom half
        // std::uniform_int_distribution<int> row_dist(rows / 2, rows - 2);
        std::uniform_int_distribution<int> col_dist(1, cols - 2);
        goal = {rows - 2, col_dist(rng)};
    }

    void GenerateMazeDFS() {
        // This guarantees a perfect maze (no loops, one path between any 2 points) by building a spanning tree
        // This method back tracks every time a dead end is encountered
        // Root = start cell

        // Assume a wall exists between 2 neighbors
        // We connect neighbors by "knocking down" the wall
        emp::vector<std::pair<int,int>> directions = {
            {0, -2}, // up
            {0, 2}, // down
            {-2, 0}, // left
            {2, 0} // right
        };

        // Start by initializing grid with walls
        grid = emp::vector<emp::vector<bool>> (rows, emp::vector<bool> (cols, true));

        // Mark starting cell as open
        // grid[start.first][start.second] = false;
        grid[rows/2][cols/2] = false;

        std::stack<std::pair<int,int>> stack;
        stack.push({rows/2, cols/2});

        // Start DFS traversal
        while (!stack.empty()) {
            std::pair<int,int> current_coord {stack.top()};
            emp::vector<std::pair<int,int>> neighbor_coords;

            // Look for unvisited neighbors
            for (auto const & [dr, dc] : directions) {
                int neighbor_r {current_coord.first + dr};
                int neighbor_c {current_coord.second + dc};

                // If neighbor is still a wall and is not past the edges of the maze
                if (neighbor_r > 0 && neighbor_r < rows - 1 &&
                    neighbor_c > 0 && neighbor_c < cols - 1 &&
                    grid[neighbor_r][neighbor_c]) {
                        neighbor_coords.emplace_back(neighbor_r, neighbor_c);
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

        // Start by initializing grid with walls
        grid = emp::vector<emp::vector<bool>> (rows, emp::vector<bool> (cols, true));

        // Mark starting cell as open
        // grid[start.first][start.second] = false;

        for (int i {1}; i < rows - 1; i += 2) {
            for (int j {1}; j < cols - 1; j += 2) {
                grid[i][j] = false;
                emp::vector<std::pair<int,int>> neighbor_coords;

                // Check for south neighbor
                if (i + 2 < rows - 1) neighbor_coords.emplace_back(i + 2, j);
                // Check for east neighbor
                if (j + 2 < cols - 1) neighbor_coords.emplace_back(i, j + 2);

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

    void GenerateMazePrim() {
        // Initialize full grid of walls
        grid = emp::vector<emp::vector<bool>>(rows, emp::vector<bool>(cols, true));

        // Directions in steps of 2 (since cells are separated by walls)
        emp::vector<std::pair<int, int>> directions = {
            {0, -2}, {0, 2}, {-2, 0}, {2, 0} // up, down, left, right
        };

        // Choose a random starting cell (odd row/col)
        std::uniform_int_distribution<int> row_dist(1, rows - 2);
        std::uniform_int_distribution<int> col_dist(1, cols - 2);
        int start_r = row_dist(rng) | 1; // force odd
        int start_c = col_dist(rng) | 1; // force odd

        std::pair<int, int> first = {start_r, start_c};
        grid[start_r][start_c] = false;

        emp::vector<std::pair<int, int>> path;
        path.push_back(first);

        // Mark all non-wall cells around starting point as unvisited
        emp::vector<std::pair<int, int>> unvisited;
        for (int r {1}; r < rows; r += 2) { // every other cell 
            for (int c {1}; c < cols; c += 2) {
                if (!(r == start_r && c == start_c)) {
                    unvisited.emplace_back(r, c);
                }
            }
        }

        while (!unvisited.empty()) {
            // Select a random cell from the current path
            std::uniform_int_distribution<size_t> dist(0, path.size() - 1);
            std::pair<int, int> current = path[dist(rng)]; 

            // Identify unvisited neighbors
            emp::vector<std::pair<int, int>> neighbors;
            for (auto const & [dr, dc] : directions) {
                int nr {current.first + dr};
                int nc {current.second + dc};
                if (nr > 0 && nr < rows - 1 && nc > 0 && nc < cols - 1) {
                    if (std::find(path.begin(), path.end(), std::make_pair(nr, nc)) == path.end()) {
                        neighbors.emplace_back(nr, nc);
                    }
                }
            }

            if (!neighbors.empty()) {
                // Choose a random neighbor
                std::uniform_int_distribution<size_t> neighbor_dist(0, neighbors.size() - 1);
                std::pair<int, int> neighbor {neighbors[neighbor_dist(rng)]};

                // Link current and neighbor
                int wall_r {(current.first + neighbor.first) / 2};
                int wall_c {(current.second + neighbor.second) / 2};
                grid[neighbor.first][neighbor.second] = false; // knock down walls
                grid[wall_r][wall_c] = false;
                path.push_back(neighbor);

                // Remove from unvisited
                auto it {std::find(unvisited.begin(), unvisited.end(), neighbor)};
                if (it != unvisited.end()) unvisited.erase(it);
            }
        }

        GenerateGoal();
    }

    // Tight paths may be too difficult for novelty search
    // Knock down some walls (half of squares in grid)
    void MakeMazeMoreOpen(/*int walls=50*/) {
        int walls {static_cast<int>((rows-1) * (cols-1) * 0.8)};
        std::uniform_int_distribution<int> row_dist(1, rows - 2);
        std::uniform_int_distribution<int> col_dist(1, cols - 2);
    
        for (int i {0}; i < walls; ++i) {
            int r {row_dist(rng)};
            int c {col_dist(rng)};
            // Turn random wall into path if it's a wall
            if (IsWall({r, c})) {
                grid[r][c] = 0;
            }
        }
    }

    // BFS
    int ComputePathLength() const {
        std::queue<std::pair<int, int>> q; // coordinate queue
        emp::vector<emp::vector<int>> dist(rows, emp::vector<int>(cols, -1));

        dist[start.first][start.second] = 0;
        q.push({start.first, start.second});
    
        while (!q.empty()) {
            int row {q.front().first};
            int col {q.front().second};
            q.pop();
    
            for (std::pair<int,int> move : moves) {
                int nrow {row + move.first};
                int ncol {col + move.second};
                if (CanMove({nrow, ncol}) && dist[nrow][ncol] == -1) {
                    dist[nrow][ncol] = dist[row][col] + 1;
                    q.push({nrow, ncol});
                }
            }
        }
        return dist[goal.first][goal.second];  // returns -1 if unreachable
    }

    emp::vector<emp::vector<bool>> GetGrid() const {
        return grid;
    }

    std::pair<int, int> GetRobotPosition() const {
        return position; // {row, col}
    }

    std::pair<int, int> GetGoalPosition() const {
        return goal;
    }

    std::pair<int, int> GetStartPosition() const {
        return start;
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

    emp::vector<double> GetSensors() const {
        return sensors;
    }

    double GetGoalAngle() const {
        int dx {goal.second - position.second}; // x is col
        int dy {goal.first - position.first}; // y is row

        double angle {std::atan2(dy, dx)}; // [-pi, pi]

        return angle / M_PI; // [-1, 1]
    }

    double GetDistToGoal() const {
        int dx {goal.second - position.second}; // col
        int dy {goal.first - position.first}; // row
        return std::hypot(dx, dy);
    }

    bool ReachedGoal() const {
        return position == goal;
    }

    // This is different from PrintMaze, which includes formatting
    void SaveMaze(std::string const & filename) const {
        std::ofstream ofs(filename);
        for (emp::vector<bool> const & row : grid) {
            for (size_t i {0}; i < row.size(); ++i) {
                ofs << row[i];
                if (i < row.size() - 1) ofs << " ";
            }
            ofs << "\n";
        }
        ofs << "START: " << start.first << " " << start.second << "\n";
        ofs << "GOAL: " << goal.first << " " << goal.second << "\n";
    }

    void LoadMaze(std::string const & filename) {
        std::ifstream ifs(filename);
        assert(ifs.is_open() && "Failed to open file: " + filename);
    
        emp::vector<emp::vector<bool>> maze;
        std::string line;
    
        while (std::getline(ifs, line)) {
            if (line.rfind("START", 0) == 0) {
                std::istringstream ss(line);
                std::string tag;
                int r, c;
                ss >> tag >> r >> c;
                start = {r, c}; // update start position
            } else if (line.rfind("GOAL", 0) == 0) {
                std::istringstream ss(line);
                std::string tag;
                int r, c;
                ss >> tag >> r >> c;
                goal = {r, c}; // update goal position
            } else {
                std::istringstream ss(line);
                emp::vector<bool> row;
                int value;
                while (ss >> value) {
                    row.push_back(static_cast<bool>(value));
                }
                if (!row.empty()) maze.push_back(row);
            }
        }
    
        grid = std::move(maze);
        rows = grid.size();
        cols = grid.empty() ? 0 : grid[0].size();
        ResetRobotPosition();
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