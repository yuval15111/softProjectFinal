#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include "Solver.h"

/*global variables the keep the current col and row we use*/
int row = 0, col = 0;

Cell** generateSudoku() {
	Cell** sudoku = (Cell**)malloc(sizeof(Cell*)*height*width);
	if (sudoku == NULL) { /*if the memory allocation didn't work*/
		printf("Error: generateSudoku has failed\n");
		exit(0);
	}
	int i, j;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			sudoku[i* width + j] = createCell(0); /*creating an empty sudoku*/
		}
	}
	return sudoku;
}

bool isRowValid(Cell** sudoku, int num) {
	int j = 0;
	for (j = 0; j < width; j++) {
		if (sudoku[(row)*width + j]->value == num) {
			return false;
		}
	}
	return true;
}

bool isColValid(Cell** sudoku, int num) {
	int i;
	for (i = 0; i < height; i++) {
		if (sudoku[i*width + col]->value == num) {
			return false;
		}
	}
	return true;
}

bool isBlockValid(Cell** sudoku, int startRow, int startCol, int num) {
	int i, j;
	for (i = 0; i < blockHeight; i++) {
		for (j = 0; j < blockWidth; j++) {
			if (sudoku[(i + startRow)*width + j + startCol]->value == num) {
				return false;
			}
		}
	}
	return true;
}

bool isValidNum(Cell** sudoku, int num) {
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
	for (row = 0; row < height; row++) {
		for (col = 0; col < width; col++) {
			if (sudoku[row*width + col]->empty == 0 && sudoku[row*width + col]->fixed == 0) {
				return true;
			}
		}
	}
	return false;
}

void stepBack() {
	col--;
	if (col < 0) {
		col = width - 1;
		row--;
	}
}

bool detBacktrackRec(Cell** sudoku) {
	int num;
	if (!findNextEmptyCell(sudoku)) {
		return true;
	}
	for (num = 1; num <= blockHeight * blockWidth; num++) { /*checks for every possible number:*/
		if (sudoku[(row)*width + col]->fixed == 0 && isValidNum(sudoku, num)) { /*if it's safe to put this num in this cell.*/
			sudoku[row*width + col]->value = num;
			sudoku[row*width + col]->empty = 1;
			if (detBacktrackRec(sudoku)) { /*if there's a solution*/
				return true;
			}
			sudoku[(row)*width + col]->value = 0; /*there's no solution with this num*/
			sudoku[row*width + col]->empty = 0;
		}
	}
	stepBack();
	while (sudoku[(row)*width + col]->fixed == 1) {/*as long as we got to a fixed cell-keep steping back*/
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
	Cell** copySudoku = (Cell**)malloc(height*width * sizeof(Cell)); /*if the memory allocation didn't work*/
	if (copySudoku == NULL) {
		printf("Error: determenisticBacktrack has failed\n");
		exit(0);
	}
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			copySudoku[i* width + j] = copyCell(currentSudoku[i* width + j]); /*copy the sudoku*/
			if (currentSudoku[i*width + j]->empty != 0) {
				copySudoku[i*width + j]->fixed = 1;
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
		boardGeneration[row*width + col]->arr[i] = 0; /*initialize the array field with 0*/
		if (isValidNum(boardGeneration, i)) {
			boardGeneration[row*width + col]->arr[i] = 1;/*insert the valid numbers to the cell's array*/
			boardGeneration[row*width + col]->arr[0] ++;
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
		if ((board[row*width + col]->arr[i]) == 1) { /*meaning we found a possible value*/
			temp--;
			if (temp <= 0) { /*we found the valid value that has been chosen*/
				board[row*width + col]->value = i;
				board[row*width + col]->empty = 1;
				board[row*width + col]->arr[i] = 0;
				board[row*width + col]->arr[0]--;
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
	while (boardGeneration[row*width + col]->arr[0] == 0) { /*there are no valid values possible for the cell*/
		boardGeneration[row*width + col]->value = 0;
		boardGeneration[row*width + col]->empty = 0;
		stepBack();
	}
	if (boardGeneration[row*width + col]->arr[0] == 1) { /*there is one valid value possible for this cell*/
		boardGeneration[row*width + col]->value = findIndexToRemove(boardGeneration[row*width + col]->arr); /*indert it*/
		boardGeneration[row*width + col]->empty = 1;
		if (randomBacktrack(boardGeneration)) { /*if we succeded*/
			return true;
		}
	}
	if (boardGeneration[row*width + col]->arr[0] > 1) { /*if there are more than one possible value for this cell*/
		removeNumberFromArray(boardGeneration, chooseRandomNumberToRemove(boardGeneration[row*width + col]->arr)); /*choose a value randomly and remove it from the array*/
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
		while (sudokoWithHints[row*width + col]->fixed == 1) { /*if fixed choose again*/
			col = rand() % (rangeOfNum);
			row = rand() % (rangeOfNum);
		}
		sudokoWithHints[row*width + col]->fixed = 1;
		sudokoWithHints[row*width + col]->empty = 1;
		sudokoWithHints[row*width + col]->value = solvedSudoku[row*width + col]->value;
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




