# normal makefile 

# Makefile for MinesweeperGame

# Compiler to use
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -g

# Linker flags
LDFLAGS =

# Source files
SOURCES = main.cpp Minesweeper.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Executable name
EXECUTABLE = MinesweeperGame

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) 