CXX=g++
# CXXFLAGS:=-Wall -Werror -Wextra -pedantic -std=c++17 -fopenmp
CXXFLAGS:=-Wall -Wextra -pedantic -std=c++17 -fopenmp
RELEASEFLAGS:=-O3
DEBUGFLAGS:=-g

.PHONY: all clean
all: submission

submission: main.o network.o
	$(CXX) $(CXXFLAGS) $(RELEASEFLAGS) -o troons $^

main.o: main.cc
	$(CXX) $(CXXFLAGS) $(RELEASEFLAGS) -c $^

network.o: network.cc
	$(CXX) $(CXXFLAGS) $(RELEASEFLAGS) -c $^

clean:
	$(RM) *.o troons

debug: main.cc network.cc
	# $(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -D DEBUG -o troons $^
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -o troons $^
