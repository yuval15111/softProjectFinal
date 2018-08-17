#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include "Solver.h"

stackNode* newNode(int row, int col) {
	stackNode* Node = (stackNode*) malloc(sizeof(stackNode));
	Node->row = row;
	Node->col = col;
	Node->lastVal = 0;
	Node->next = NULL;
	return Node;
}

void push(stackNode** root, int row, int col, int val) {
	stackNode* Node = newNode(row, col);
	Node->lastVal = val;
	Node->next = *root;
	*root = Node;
}

int isEmpty(stackNode* root) {
	if (root == NULL) {
		return 0;
	}
	else return 1;
}

int* pop(stackNode** root) {
	stackNode* temp;
	int popped[3] = { -1,-1,-1 };
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

int* peek(stackNode* root) {
	int popped[3] = { -1,-1,-1 };
	if (!isEmpty(root)) {
		return popped;
	}
	popped[0] = root->row;
	popped[1] = root->col;
	popped[2] = root->lastVal;
	return popped;
}

void findNextEmptyCell(Cell** sudoku, int row, int col, int* index) {
	int i, j;
	index[0] = -1;
	index[1] = -1;
	for (i=row; i < N; i++) {
		for (j=0; j < N; j++) {
			if (sudoku[i*N + j]->empty == 0 && sudoku[i*N + j]->fixed == 0) {
				if ((i == row && j > col) || (i > row)) {
					index[0] = i;
					index[1] = j;
					return;
				}
			}
		}
	}
	return;
}

int findNextVal(Cell** sudoku, int row, int col, int curVal) {
	int tempVal = (curVal+1);
	for (tempVal; tempVal <= N; tempVal++) {
		if ((isRowValidGame(sudoku, row, col, tempVal)) && (isColValidGame(sudoku, row, col, tempVal)) && (isBlockValidGame(sudoku, row - (row%blockHeight), col - (col%blockWidth),row ,col, tempVal))) {
			return tempVal;
		}
	}
	return -1;
}

int exBackTrac(Cell** sudoku) {
	int solutionCounter = 0, flag = 1, curRow = 0, curCol = 0, nextVal = 0;
	int index[2] = { -1,-1 }, *popped;
	int check = 0;
	stackNode* root = NULL;
	push(&root, -1, -1, -1);
	findNextEmptyCell(sudoku, curRow, curCol, index);
	curCol = index[1];
	curRow = index[0];
	while (flag) {
		if (index[0] == -1) {  
			solutionCounter++;
			popped = pop(&root); 
			if (root->col == -1) {
				flag = 0; 
				break;
			}
			sudoku[popped[0]*N + popped[1]]->value = 0;
			sudoku[popped[0]*N + popped[1]]->empty = 0;
			index[0] = popped[0];
			index[1] = popped[1];
			popped = peek(root);
			curCol = popped[1];
			curRow = popped[0];
		}

		/* find the next value (value+1) for the curr cell. */
		nextVal = findNextVal(sudoku, curRow, curCol, sudoku[curRow*N + curCol]->value);
		if (nextVal == -1) { /*if there is not valid number*/
			popped = peek(root);
			if (popped[0] == curRow && popped[1] == curCol) { /* check if the cell is in the stack, means that we need to popped because there is no a valid number for this cell*/
				popped = pop(&root);
				if (root->col == -1) {
					flag = 0;
					break;
				}
				/* initialize the cell to be 0 */
				sudoku[popped[0]*N + popped[1]]->value = 0;
				sudoku[popped[0]*N + popped[1]]->empty = 0;

				popped = peek(root);
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
	return solutionCounter;
}





