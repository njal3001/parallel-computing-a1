CXX=g++
# CXXFLAGS:=-Wall -Werror -Wextra -pedantic -std=c++17 -fopenmp
CXXFLAGS:=-Wall -Wextra -pedantic -std=c++17 -fopenmp
RELEASEFLAGS:=-O3
# RELEASEFLAGS:=-g3
DEBUGFLAGS:=-g

.PHONY: all clean
all: submission

submission: main.cc network.cc
	$(CXX) $(CXXFLAGS) $(RELEASEFLAGS) -o troons $^

clean:
	$(RM) *.o troons

debug: main.cc network.cc network.h
	# $(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -D DEBUG -o troons $^
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -o troons $^
