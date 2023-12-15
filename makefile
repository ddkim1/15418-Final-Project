# normal makefile 

# Makefile for MinesweeperGame

# Compiler to use
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -g -fopenmp

# Linker flags
LDFLAGS = -fopenmp

# Source files
SOURCES = main.cpp sequential-minesweeper.cpp openmp-minesweeper.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Executable name
EXECUTABLE = minesweeper

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) 
