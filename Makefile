# Compiler and flags
CXX := c++
# DNDEBUG = turns off all asserts - faster to run
CXXFLAGS := -std=c++20 -Wall -Wextra -O3 -I../Empirical/include -DNDEBUG
# CXXFLAGS := -std=c++20 -Wall -Wextra -g -I../Empirical/include
# CXXFLAGS := -std=c++20 -Wall -Wextra -g 

# Target name
TARGET := KarLGP

# Source and object files
SRCS := maze/maze_test.cpp
OBJS := $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile each .cpp file into a .o file
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(OBJS) $(TARGET)