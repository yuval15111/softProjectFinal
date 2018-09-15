#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include "Solver.h"

/*
This function gets a row and col and adds a new node to a doubly linked list.
it insert row and col to be the fields values.
*/
stackNode* newNode(int row, int col) {
	stackNode* Node = (stackNode*) malloc(sizeof(stackNode));
	Node->row = row;
	Node->col = col;
	Node->lastVal = 0;
	Node->next = NULL;
	return Node;
}

/*
This function insert a new node to the begining of a stack.
it gets the root of the stack, a row, col and value.
it creates a new node using 'newNode' function,
it updates the new node's fields and place it as the new root.
*/
void push(stackNode** root, int row, int col, int val) {
	stackNode* Node = newNode(row, col);
	Node->lastVal = val;
	Node->next = *root;
	*root = Node;
}

/*
This function gets the root of the stuck and checks wether rhe root is NULL.
@return
0 -> if the stack is empty (the root is NULL)
1 -> else
*/
int isEmpty(stackNode* root) {
	if (root == NULL) {
		return 0;
	}
	else return 1;
}

/*
This function gets a pointer to the root of the stack, delete the first node and return it.
@return
the root of the stack
*/
int* pop(stackNode** root) {
	stackNode* temp;
	int* popped = (int*)malloc(sizeof(int) * 3);
	popped[0] = -1;
	popped[1] = -1;
	popped[2] = -1;
	if (!isEmpty(*root)) {
		return popped;
	}
	temp = *root;
	*root = (*root)->next;
	popped[0] = temp->row;
	popped[1] = temp->col;
	popped[2] = temp->lastVal;
	free(temp);
	return popped;
}

/*
This function gets the root of the stack and return it's values.
@return
an int array of size 3 that includes the row, col and last value of the root.
*/
int* peek(stackNode* root) {
	int* popped = (int*)malloc(sizeof(int) * 3);
	popped[0] = -1;
	popped[1] = -1;
	popped[2] = -1;
	if (!isEmpty(root)) {
		return popped;
	}
	popped[0] = root->row;
	popped[1] = root->col;
	popped[2] = root->lastVal;
	return popped;
}

/*
This function finds the next empty cell in the sudoku.
it gets a sudoku, row, col and a pointer to int => index.
it goes over all the cells and checks if the cell is empty and
if the empty cell it found is after the cell it got in the command line.
if so- it updates index[0]=row of the empty cell it found, index[1]=column of the empty cell it found.
*/
void findNextEmptyCell(Cell** sudoku, int row, int col, int* index) {
	int i, j;
	index[0] = -1;
	index[1] = -1;
	for (i=row; i < N; i++) {
		for (j=0; j < N; j++) {
			if (sudoku[i*N + j]->empty == 0 && sudoku[i*N + j]->fixed == 0) {
				if ((i == row && j > col) || (i > row)) { /*if the cell is after the current one (next empty)*/
					index[0] = i;
					index[1] = j;
					return;
				}
			}
		}
	}
	return;
}

/*
This function finds the next valid value for a cell.
it gets a sudoku, row, col and current value.
it goes over all the numbers between curVal+1 and N and checks if the value is valid
using 'isRowValidGame', 'isColValidGame', 'isBlockValidGame' functions.
@return
-1 -> no valid value has been found
else -> the valid value
*/
int findNextVal(Cell** sudoku, int row, int col, int curVal) {
	int tempVal = (curVal+1); /*the next value for the current cell*/
	for (tempVal; tempVal <= N; tempVal++) {/*checks validation*/
		if ((isRowValidGame(sudoku, row, col, tempVal)) && (isColValidGame(sudoku, row, col, tempVal)) && (isBlockValidGame(sudoku, row - (row%blockWidth), col - (col%blockHeight),row ,col, tempVal))) {
			return tempVal;
		}
	}
	return -1;/*there is no valid next value*/
}

int exBackTrack(Cell** sudoku) {
	int solutionCounter = 0, flag = 1, curRow = 0, curCol = 0, nextVal = 0;
	int index[2] = {-1, -1}, *poppedMalloc, *popped;
	int temp[3] = {-1, -1, -1};
	int check = 0;
	stackNode* root = NULL;
	push(&root, -1, -1, -1);
	findNextEmptyCell(sudoku, curRow, curCol, index);
	curCol = index[1];
	curRow = index[0];
	while (flag) {
		if (index[0] == -1) {  /*the is no next empty cell -> we found a solution*/
			solutionCounter++;
			poppedMalloc = pop(&root);
			temp[0] = poppedMalloc[0];
			temp[1] = poppedMalloc[1];
			temp[2] = poppedMalloc[2];
			free(poppedMalloc);
			popped = temp;
			if (root->col == -1) {
				flag = 0; 
				break;
			}
			sudoku[popped[0]*N + popped[1]]->value = 0;
			sudoku[popped[0]*N + popped[1]]->empty = 0;
			index[0] = popped[0];
			index[1] = popped[1];
			poppedMalloc = peek(root);
			temp[0] = poppedMalloc[0];
			temp[1] = poppedMalloc[1];
			temp[2] = poppedMalloc[2];
			free(poppedMalloc);
			popped = temp;
			curCol = popped[1];
			curRow = popped[0];
		}
		/* find the next value (value+1) for the curr cell. */
		nextVal = findNextVal(sudoku, curRow, curCol, sudoku[curRow*N + curCol]->value);
		if (nextVal == -1) { /*if there is not valid number*/
			poppedMalloc = peek(root);
			temp[0] = poppedMalloc[0];
			temp[1] = poppedMalloc[1];
			temp[2] = poppedMalloc[2];
			free(poppedMalloc);
			popped = temp;
			if (popped[0] == curRow && popped[1] == curCol) { /* check if the cell is in the stack, means that we need to popped because there is no a valid number for this cell*/
				poppedMalloc = pop(&root);
				temp[0] = poppedMalloc[0];
				temp[1] = poppedMalloc[1];
				temp[2] = poppedMalloc[2];
				free(poppedMalloc);
				popped = temp;
				if (root->col == -1) {
					flag = 0;
					break;
				}
				/* initialize the cell to be 0 */
				sudoku[popped[0]*N + popped[1]]->value = 0;
				sudoku[popped[0]*N + popped[1]]->empty = 0;
				poppedMalloc = peek(root);
				temp[0] = poppedMalloc[0];
				temp[1] = poppedMalloc[1];
				temp[2] = poppedMalloc[2];
				free(poppedMalloc);
				popped = temp;
			}
			/* we stepBack to the prev cell and we will find for this cell the next valid number (if there is)*/
			curRow = popped[0];
			curCol = popped[1];
			continue;
		}
		else {
			sudoku[curRow*N + curCol]->value = nextVal;
			sudoku[curRow*N + curCol]->empty = 1;
			if (root->col == curCol && root->row == curRow) {
				root->lastVal = nextVal;
			}
			else {
				push(&root, curRow, curCol, nextVal);
			}
		}
		findNextEmptyCell(sudoku, curRow, curCol,index);
		curRow = index[0];
		curCol = index[1];
		if (root->col == -1 && root->row == -1) {
			flag = 0;
			break;
		}
	}
	while (isEmpty(root)) {
		pop(&root);
	}
	return solutionCounter;
}