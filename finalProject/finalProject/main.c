#include<time.h>
#include<stdio.h>
#include"Solver.h"
#include <stdlib.h>
#include "Parser.h"

int main(int argc, char* argv[]) {
	int seed;
	if (argc == 2) {
		seed = atoi(argv[1]);
		srand(seed);
		printf("Sudoku\n------\n");
		puzzleGeneration(initNumberOfHints());
		playGame();
	}
	return 0;
}