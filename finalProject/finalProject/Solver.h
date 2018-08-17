/*
The module solver implements the Backtrack algorithms.
In addition, this module generates the sudoku boards without any connection to the user.
*/

#ifndef SOLVER_H_
#define SOLVER_H_
#include<stdbool.h>
#include "Game.h"

typedef struct stackNode {/* definition of the stack's nodes */
	int row, col, lastVal;
	struct stackNode* next;
}stackNode;


void findNextEmptyCell(Cell** sudoku, int row, int col, int* c);
int exBackTrac(Cell** sudoku);

#endif /*SOLVER_H_*/

