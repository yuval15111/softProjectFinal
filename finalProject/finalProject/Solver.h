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
This function gets a row and col and adds a new node to a doubly linked list.
it insert row and col to be the fields values.
*/
stackNode* newNode(int row, int col);

/*
This function insert a new node to the begining of a stack.
it gets the root of the stack, a row, col and value.
it creates a new node using 'newNode' function,
it updates the new node's fields and place it as the new root.
*/
void push(stackNode** root, int row, int col, int val);

/*
This function gets the root of the stuck and checks wether rhe root is NULL.
@return
0 -> if the stack is empty (the root is NULL)
1 -> else
*/
int isEmpty(stackNode* root);

/*
This function gets a pointer to the root of the stack, delete the first node and return it.
@return
the root of the stack
*/
int* pop(stackNode** root);

/*
This function gets the root of the stack and return it's values.
@return
an int array of size 3 that includes the row, col and last value of the root.
*/
int* peek(stackNode* root);

/*
This function finds the next empty cell in the sudoku.
it gets a sudoku, row, col and a pointer to int => index.
it goes over all the cells and checks if the cell is empty and 
if the empty cell it found is after the cell it got in the command line.
if so- it updates index[0]=row of the empty cell it found, index[1]=column of the empty cell it found.
*/
void findNextEmptyCell(Cell** sudoku, int row, int col, int* c);

/*
This function finds the next valid value for a cell.
it gets a sudoku, row, col and current value.
it goes over all the numbers between curVal+1 and N and checks if the value is valid
using 'isRowValidGame', 'isColValidGame', 'isBlockValidGame' functions.
@return
-1 -> no valid value has been found
else -> the valid value
*/
int findNextVal(Cell** sudoku, int row, int col, int curVal);

/*
#################3not ready yet##################
This function implements the exhausting backTrack algorithm.
it gets a sudoku and return the number of solutions it has.

*/
int exBackTrack(Cell** sudoku);

#endif /*SOLVER_H_*/

