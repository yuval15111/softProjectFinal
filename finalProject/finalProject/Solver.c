#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include "Solver.h"


/*global variables the keep the current col and row we use*/
int row = 0, col = 0;

Cell** generateSudoku() {
	Cell** sudoku = (Cell**)malloc(sizeof(Cell*)*N*N);
	if (sudoku == NULL) { /*if the memory allocation didn't work*/
		printf("Error: generateSudoku has failed\n");
		exit(0);
	}
	int i, j;
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			sudoku[i* N + j] = createCell(0); /*creating an empty sudoku*/
		}
	}
	return sudoku;
}

bool isRowValid(Cell** sudoku, int num) {/*#####################################delete ##############################*/
	int j = 0;
	for (j = 0; j < N; j++) {
		if (sudoku[(row)*N + j]->value == num) {
			return false;
		}
	}
	return true;
}

bool isColValid(Cell** sudoku, int num) {/*#####################################delete ##############################*/
	int i;
	for (i = 0; i < N; i++) {
		if (sudoku[i*N + col]->value == num) {
			return false;
		}
	}
	return true;
}

bool isBlockValid(Cell** sudoku, int startRow, int startCol, int num) {/*##################delete ######################*/
	int i, j;
	for (i = 0; i < blockHeight; i++) {
		for (j = 0; j < blockWidth; j++) {
			if (sudoku[(i + startRow)*N + j + startCol]->value == num) {
				return false;
			}
		}
	}
	return true;
}

bool isValidNum(Cell** sudoku, int num) {/*#####################################delete ##############################*/
	bool rowValid, colValid, blockValid;
	rowValid = isRowValid(sudoku, num);
	colValid = isColValid(sudoku, num);
	blockValid = isBlockValid(sudoku, row - row % blockWidth, col - col % blockHeight, num);
	if (rowValid && colValid && blockValid) {
		return true;
	}
	return false;
}

bool findNextEmptyCell(Cell** sudoku) {
	for (row = 0; row < N; row++) {
		for (col = 0; col < N; col++) {
			if (sudoku[row*N + col]->empty == 0 && sudoku[row*N + col]->fixed == 0) {
				return true;
			}
		}
	}
	return false;
}

void stepBack() {
	col--;
	if (col < 0) {
		col = N - 1;
		row--;
	}
}

bool detBacktrackRec(Cell** sudoku) {
	int num;
	if (!findNextEmptyCell(sudoku)) {
		return true;
	}
	for (num = 1; num <= blockHeight * blockWidth; num++) { /*checks for every possible number:*/
		if (sudoku[(row)*N + col]->fixed == 0 && isValidNum(sudoku, num)) { /*if it's safe to put this num in this cell.*/
			sudoku[row*N + col]->value = num;
			sudoku[row*N + col]->empty = 1;
			if (detBacktrackRec(sudoku)) { /*if there's a solution*/
				return true;
			}
			sudoku[(row)*N + col]->value = 0; /*there's no solution with this num*/
			sudoku[row*N + col]->empty = 0;
		}
	}
	stepBack();
	while (sudoku[(row)*N + col]->fixed == 1) {/*as long as we got to a fixed cell-keep steping back*/
		if (row == 0 && col == 0) {
			return false;
		}
		stepBack();
	}
	return false; /*there's no solution at all*/
}

Cell** determenisticBacktrack(Cell** currentSudoku) {
	int i = 0, j;
	bool flag = false;
	Cell** copySudoku = (Cell**)malloc(N*N * sizeof(Cell)); /*if the memory allocation didn't work*/
	if (copySudoku == NULL) {
		printf("Error: determenisticBacktrack has failed\n");
		exit(0);
	}
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			copySudoku[i* N + j] = copyCell(currentSudoku[i* N + j]); /*copy the sudoku*/
			if (currentSudoku[i*N + j]->empty != 0) {
				copySudoku[i*N + j]->fixed = 1;
			}
		}
	}
	flag = detBacktrackRec(copySudoku);
	if (flag) { /*if there is a sulotion*/
		return copySudoku;
	}
	else {
		freeSudoku(copySudoku);
		return NULL;
	}
}

void updateArrayForCell(Cell** boardGeneration) {
	int i;
	for (i = 1; i <= blockHeight * blockWidth; i++) {
		boardGeneration[row*N + col]->arr[i] = 0; /*initialize the array field with 0*/
		if (isValidNum(boardGeneration, i)) {
			boardGeneration[row*N + col]->arr[i] = 1;/*insert the valid numbers to the cell's array*/
			boardGeneration[row*N + col]->arr[0] ++;
		}
	}
}

int chooseRandomNumberToRemove(int* arrayOfNumbers) {
	return (rand() % arrayOfNumbers[0]);
}

void removeNumberFromArray(Cell** board, int rand) {
	int temp = rand + 1;
	int i;
	for (i = 1; i <= blockHeight * blockWidth; i++) {
		if ((board[row*N + col]->arr[i]) == 1) { /*meaning we found a possible value*/
			temp--;
			if (temp <= 0) { /*we found the valid value that has been chosen*/
				board[row*N + col]->value = i;
				board[row*N + col]->empty = 1;
				board[row*N + col]->arr[i] = 0;
				board[row*N + col]->arr[0]--;
				break;
			}
		}
	}
}

int findIndexToRemove(int* arr) {
	int i;
	for (i = 1; i <= blockHeight * blockWidth; i++) {
		if (arr[i] == 1) { /*we found the index of the only possible value*/
			break;
		}
	}
	arr[0] = 0; /*there are no more valid values*/
	arr[i] = 0;
	return i;
}

bool randomBacktrack(Cell** boardGeneration) {
	if (!findNextEmptyCell(boardGeneration)) {
		return true;
	}
	updateArrayForCell(boardGeneration);
	while (boardGeneration[row*N + col]->arr[0] == 0) { /*there are no valid values possible for the cell*/
		boardGeneration[row*N + col]->value = 0;
		boardGeneration[row*N + col]->empty = 0;
		stepBack();
	}
	if (boardGeneration[row*N + col]->arr[0] == 1) { /*there is one valid value possible for this cell*/
		boardGeneration[row*N + col]->value = findIndexToRemove(boardGeneration[row*N + col]->arr); /*indert it*/
		boardGeneration[row*N + col]->empty = 1;
		if (randomBacktrack(boardGeneration)) { /*if we succeded*/
			return true;
		}
	}
	if (boardGeneration[row*N + col]->arr[0] > 1) { /*if there are more than one possible value for this cell*/
		removeNumberFromArray(boardGeneration, chooseRandomNumberToRemove(boardGeneration[row*N + col]->arr)); /*choose a value randomly and remove it from the array*/
		if (randomBacktrack(boardGeneration)) { /*if we succeded*/
			return true;
		}
	}
	return false;
}

void getHintsBoard(int hints, Cell** solvedSudoku, Cell** sudokoWithHints) {
	int i, row, col;
	for (i = 0; i < hints; i++) {
		col = rand() % (rangeOfNum);
		row = rand() % (rangeOfNum);
		while (sudokoWithHints[row*N + col]->fixed == 1) { /*if fixed choose again*/
			col = rand() % (rangeOfNum);
			row = rand() % (rangeOfNum);
		}
		sudokoWithHints[row*N + col]->fixed = 1;
		sudokoWithHints[row*N + col]->empty = 1;
		sudokoWithHints[row*N + col]->value = solvedSudoku[row*N + col]->value;
	}
}

void puzzleGeneration(int hints) {
	Cell** boardGeneration = generateSudoku();
	Cell** sudokoWithHints = generateSudoku();
	randomBacktrack(boardGeneration);
	getSolvedSudoku(boardGeneration);
	getHintsBoard(hints, boardGeneration, sudokoWithHints);
	getSudokuWithHints(sudokoWithHints);
	printSudoku(sudokoWithHints);
}


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

void findNextEmptyCell2(Cell** sudoku, int row, int col, int* index) {
	index[0] = -1;
	index[1] = -1;
	for (int i=row; i < N; i++) {
		for (int j=0; j < N; j++) {
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
	findNextEmptyCell2(sudoku, curRow, curCol, index);
	curCol = index[1];
	curRow = index[0];
	while (flag) {
		if (index[0] == -1) {  // find solution for the sudoku
			solutionCounter++;
			popped = pop(&root); // if we got to the first cell in board
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

		// find the next value (value+1) for the curr cell. 
		nextVal = findNextVal(sudoku, curRow, curCol, sudoku[curRow*N + curCol]->value);
		if (nextVal == -1) { //if there is not valid number
			popped = peek(root);
			if (popped[0] == curRow && popped[1] == curCol) { // check if the cell is in the stack, means that we need to popped because there is no a valid number for this cell
				popped = pop(&root);
				if (root->col == -1) {
					flag = 0;
					break;
				}
				// initialize the cell to be 0
				sudoku[popped[0]*N + popped[1]]->value = 0;
				sudoku[popped[0]*N + popped[1]]->empty = 0;
				//index[0] = popped[0];
				//index[1] = popped[1];
				popped = peek(root);
			}
			// we stepBack to the prev cell and we will find for this cell the next valid number (if there is)
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
		findNextEmptyCell2(sudoku, curRow, curCol,index);
		curRow = index[0];
		curCol = index[1];
		if (root->col == -1 && root->row == -1) {
			flag = 0;
			break;
		}
	}
	return solutionCounter;
}





