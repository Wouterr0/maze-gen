CPPFLAGS = -Wall -Wextra -std=c++17 -pedantic -ggdb
SRC = maze.cpp
LIBS = -lsfml-graphics -lsfml-window -lsfml-system

maze: $(SRC)
	g++ $(CPPFLAGS) -o maze $(SRC) $(LIBS)