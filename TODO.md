# To do list

- Storing global parameters in a single .hpp file may not be the best strategy
- Actually implement read-only registers (for input). Right now I'm just labeling them
- Add support for unequal-length programs?
- By default, output is currently register 0, input is register 1. Realistically, program length shouldn't be less than 2, but I should implement safeguards
- Support methods that require multiple fitness values (e.g., lexicase selection)
- Crossover and mutation are currently combined under a single Variator class. In experiment files, the order in which they are added determines the order in which they are applied. 
- Support for crossover that produces more than one child?
- Right now the entire population is replaced/gen, not very flexible and may be too extreme. 
- ~~Verbose mode currently only tracks the best fitness~~
- Evolution is currently purely generational
- Population size is currently fixed
- For separate experiments, I'm creating new config files to specify parameters. Probably not the best long-term solution.
- Implement save/load functionality for runs
- Implement save/load functionality for individual programs
- Representation is currently purely register-based
- Is there a better approach to clamping in fitness calculations and registers?
- Support for random ephemeral constants
- Support for instructions involving 2 constants
- Add Boolean logic/control flow instructions
- A way to demonstrate the program in execution so that the users would not have to do analysis themselves
- Better printing for unary operators
- ~~Median more meaningful than avg if fitness not normally distributed~~
- Geometric mean: exponential skew, take log of all vals find mean of that then re exponentiate (make sure same base as when log)
- NUMBER 1: Start your program with a perfect solution, make sure it does not evolve away (if it does, error in system :/)
- if large scale, maybe dont have global variables
- MAKE EVERYTHING CONSISTENT - arith prog doesnt support ternary operators, but maze prog does
- change std::vector to emp::vector (so far, just maze_prog and operators)
- novelty archive threshold (p_min) is currently static, not dynamic
- limit archive size
- refactor code for better performance and coding practices because right now everything is a mess
- 4/24: changed fitness to be maximizing instead of minimizing.
    - MSE Eval needs to be updated

## Maze Navigation Domain
- Implement other kinds of maze generation algorithms
    - e.g. binary tree
    - Has to generate perfect mazes, because guarantees there's a path between any two start-goal cells
    - How to tell which method produces more deceptive mazes?
    - DFS backtracks, may have a tendency to create paths that circle back towards the middle of the maze?
        - Or long winding paths
    - Observation: DFS-generated mazes are very easy when small (<= 7x7)
    - Binary tree might be a good idea, because to generate paths it always moves down and right
        - as long as goal aligns with starting point, this usually creates a deceptive maze
        - Downside is more deterministic/predictable shape
- MazeEval: allow for a test set of mazes with different dimensions
- I wonder if including angle to goal in sensor will have an effect on evolution

- Testing:
    - Does MazeProgram work with MazeEnv?
    
