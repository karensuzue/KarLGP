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
