/*
The module solver implements the Backtrack algorithms.
In addition, this module generates the sudoku boards without any connection to the user.
*/

#ifndef SOLVER_H_
#define SOLVER_H_
#include<stdbool.h>
#include "Game.h"

/*
This struct implements a node of a stack.
*/
typedef struct stackNode {
	int row, col, lastVal;
	struct stackNode* next;
}stackNode;

/*
This function implements the exhausting backTrack algorithm.
it gets a sudoku and return the number of solutions it has by implementing a stack using stackNode.

*/
int exBackTrack(Cell** sudoku);

#endif /*SOLVER_H_*/

