'core': Base classes? User can extend to define their own 
'estimator.hpp': Big encapsulating class. Goal: estimate target program. Calls all the other classes and performs the evolutionary run, similar to PyghGP. you set the seed here.

'global.hpp' holds global parameter values 
All test files has to include 'core/global.hpp', but nothing else
None of the other program.hpp files have to include global.hpp, wouldnt work anyway cause circular dependency

classes with 'base' are meant to be inherited and extended 


TODOs:
- global parameters in a single .hpp file may not be the best strategy
- Actually implement read-only register (for input). Right now i'm just labeling them 
- support for unequal length programs?
- output is register 0, input is register 1. realistically, program length shouldn't be less than 2, but i should implement safeguards
- support for methods that require multiple fitness values, like lexicase selection
- Crossover and mutation are currently combined under one Variator class, so order of application is very important
    - allows for flexibility? but maybe not entirely necessary 
- verbose only keeps track of best fitness
- currently purely generational
- currently fixed pop size
- eval modules are kinda connected to the "env" concept
- for separate experiments, make a new "global.hpp" file to specify your parameters. probably not the best way to store these things though
- save/load runs
- save/load programs
- currently purely register based represdentation

MAZE NAVIGATION PROBLEM:
