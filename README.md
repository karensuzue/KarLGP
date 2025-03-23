'core': Base classes? User can extend to define their own 
'estimator.hpp': Big encapsulating class. Goal: estimate target program. Calls all the other classes and performs the evolutionary run, similar to PyghGP. you set the seed here.

'global.hpp' holds global variable values 
All test files has to include 'core/global.hpp', but nothing else
None of the other program.hpp files have to include global.hpp, wouldnt work anyway cause circular dependency

classes with 'base' are meant to be inherited and extended 


TODOs:
- Actually implement read-only register (for input). Right now i'm just labeling them 