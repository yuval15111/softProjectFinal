#include<time.h>
#include<stdio.h>
#include"Game.h"
#include <stdlib.h>

int main(int argc, char* argv[]) {
	int seed;
	seed = time(NULL);
	srand(seed);
	printf("Sudoku\n------\n");
	playGame();
	return 0;
}