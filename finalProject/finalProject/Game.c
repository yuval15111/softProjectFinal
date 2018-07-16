#include<stdio.h>
#include<stdlib.h>
#include <fcntl.h>
#include "Solver.h"
#include "Parser.h"

int markError = 1;
mode = 2; /*1 - solve mode and 0 - edit mode and 2 - init*/
/*n, m,N - n=num of rows of blocks, m=num of columns of blocks*/
linkedList undo_redo;


/*
solvedSudoku is the solution of the board from the last time the user checked the validaty of the board
*/
Cell** solvedSudoku;

/*
currentSudoku is the board that the user is playing on
*/
Cell** currentSudoku;

void initList(linkedList list) {
	list.head = NULL;
	list.current = NULL;
}

void freeList(node* node) {
	if (node == NULL)
		return;
	else {
		return freeList(node->next);
		free(node);
	}
}

void addNode(linkedList* list, int row, int col, int val, int oldVal) {
	node* newNode = (node*)malloc((sizeof(node)));
	if (newNode == NULL) { /*the memory allocation failed*/
		exitGame();
	}
	newNode->col = col;
	newNode->row = row;
	newNode->value = val;
	newNode->oldValue = oldVal;
	newNode->next = list->head;
	//temp = list->head;
	list->head = newNode;
	newNode->prev = list->head->prev;
	//list->head = newNode;
	list->current = newNode;
}

Cell* createCell(int value) {
	Cell* cell = (Cell*)malloc(sizeof(Cell));
	if (cell == NULL) {
		printf("Error: createCell has failed\n");
		exit(0);
	}
	cell->value = value;
	cell->fixed = 0;
	cell->empty = 0;
	cell->arr[0] = 0;
	cell->erroneous = 0;
	initList(cell->erroneousNeib);
	return cell;
}

Cell* copyCell(Cell* cell) {
	Cell* newCell = (Cell*)malloc(sizeof(Cell));
	if (newCell == NULL) { /*the memory allocation didn't work*/
		printf("Error: copyCell has failed\n");
		exit(0);
	}
	newCell->empty = cell->empty;
	newCell->fixed = cell->fixed;
	newCell->value = cell->value;
	newCell->erroneous = cell->erroneous;
	newCell->erroneousNeib = cell->erroneousNeib;
	return newCell;

}

void printSeparator() {
	for (int i = 0; i < 4 * N + blockWidth + 1; i++) {
		printf("-");
	}
	printf("\n");
}

void printSudoku(Cell** sudoku) {
	int i, j, k, s;
	printSeparator();
	for (i=0; i <blockWidth; i++) { /*rows of blocks*/
		for (j=0; j < blockHeight; j++) { /*rows in the block*/
			for (s=0; s<blockHeight;s++){ /*cols of blocks*/
				printf("|");
				for (k=0; k < blockWidth; k++) { /*cols in the block*/
					printf(" ");
					if (sudoku[i*blockHeight*N + j * N + s * blockWidth + k]->empty == 0) {
						printf("   ");
					}
					else {
						printf("%2d", sudoku[i*blockHeight*N + j * N + s * blockWidth + k]->value);
						if (sudoku[i*blockHeight*N + j * N + s * blockWidth + k]->fixed == 1) {
							printf(".");
						}
						else if (sudoku[i*blockHeight*N + j * N + s * blockWidth + k]->erroneous == 1) {
							if (mode == 2 || markError == 1) {
								printf("*");
							}
							else {
								printf(" ");
							}
						}
						else {
							printf(" ");
						}
					}
				}
			}
			printf("|\n");
		}
		printSeparator();
	}
}


bool isRowValidGame(Cell** sudoku, int row, int col, int num) {
	int j = 0;
	for (j = 0; j < N; j++) {
		if (sudoku[(row)*N + j]->value == num) {
			if (j != col) {
				return false;
			}
		}
	}
	return true;
}

bool isColValidGame(Cell** sudoku, int row, int col, int num) {
	int i;
	for (i = 0; i < N; i++) {
		if (sudoku[i*N + col]->value == num) {
			if (i != row) {
				return false;
			}
		}
	}
	return true;
}

bool isBlockValidGame(Cell** sudoku, int startRow, int startCol, int row, int col, int num) {
	int i, j;
	for (i = 0; i < blockHeight; i++) {
		for (j = 0; j < blockWidth; j++) {
			if (sudoku[(i + startRow)*N + j + startCol]->value == num) {
				if (row != i + startRow || col != j + startCol) {
					return false;
				}
			}
		}
	}
	return true;
}

void validate(Cell** currentSudoku) {
	Cell** newSolvedSudoku;
	newSolvedSudoku = determenisticBacktrack(currentSudoku);
	if (newSolvedSudoku == NULL) {
		printf("Validation failed: board is unsolvable\n");
	}
	else {
		freeSudoku(solvedSudoku);
		solvedSudoku = newSolvedSudoku;
		printf("Validation passed: board is solvable\n");
	}
}

void hint(int row, int col) {
	printf("Hint: set cell to %d\n", solvedSudoku[row*N + col]->value);
}

bool isGameOver(Cell** sudoku) {
	int i, j;
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			if (sudoku[i*N + j]->empty == 0) {
				return false;
			}
		}
	}
	return true;
}

void checkErrorRow(int row, int col, int val) {
	for (int j = 0; j < N; j++) { 
		if (currentSudoku[(row)*N + j]->value == val) {
			if (j != col) {
				currentSudoku[row*N + col]->erroneous = 1;
				addNode(&currentSudoku[row*N + col]->erroneousNeib, row, j, val, 0);
				if (currentSudoku[row*N + j]->fixed == 0) {
					currentSudoku[row*N + j]->erroneous = 1;
					addNode(&currentSudoku[row*N + j]->erroneousNeib, row, col, val, 0);
				}
			}
		}
	}
}

void checkErrorCol(int row, int col, int val) {
	for (int j = 0; j < N; j++) {
		if (currentSudoku[j*N + col]->value == val) {
			if (j != row) {
				currentSudoku[row*N + col]->erroneous = 1;
				addNode(&currentSudoku[row*N + col]->erroneousNeib, j, col, val, 0);
				if (currentSudoku[j*N + col]->fixed == 0) {
					currentSudoku[j*N + col]->erroneous = 1;
					addNode(&currentSudoku[j*N + col]->erroneousNeib, row, col, val, 0);
				}
			}
		}
	}
}

void checkErrorBlock(int row, int col, int val) {
	int startRow = row - row % blockWidth;
	int startCol = col - col % blockHeight;
	for (int i = 0; i < blockHeight; i++) {
		for (int j = 0; j < blockWidth; j++) {
			if (currentSudoku[(i + startRow)*N + j + startCol]->value == val) {
				if (row != i + startRow || col != j + startCol) {
					currentSudoku[row*N + col]->erroneous = 1;
					addNode(&currentSudoku[row*N + col]->erroneousNeib, i + startRow, j + startCol, val, 0);
					if (currentSudoku[(i+startRow)*N + j+startCol]->fixed == 0) {
						currentSudoku[(i + startRow)*N + j + startCol]->erroneous = 1;
						addNode(&currentSudoku[(i + startRow)*N + j + startCol]->erroneousNeib, row, col, val, 0);
					}
				}
			}
		}
	}
}

void deleteNode(Cell* cell, linkedList* list ,node* origNode) {
	if (origNode->next->row == origNode->row && origNode->next->col == origNode->col) {
		free(origNode);
		origNode = NULL;
	}
	else {
		if ((list.head->row == origNode->row) && (list.head->col == origNode->col)) {
			list.head = list.head->next;
		}
		origNode->prev->next = origNode->next;
		origNode->next->prev = origNode->prev;
		free(origNode);
	}
}

void erroneousFixDel(int row, int col, int val) {
	int tempRow, tempCol;
	while (currentSudoku[row*N + col]->erroneousNeib.head != NULL) {
		tempRow = currentSudoku[row*N + col]->erroneousNeib.head->row;
		tempCol = currentSudoku[row*N + col]->erroneousNeib.head->col;
		currentSudoku[tempRow*N + tempCol]->erroneousNeib.current = currentSudoku[tempRow*N + tempCol]->erroneousNeib.head;
		while (currentSudoku[tempRow*N + tempCol]->erroneousNeib.current->row != row || /*Looking for the main node*/
			currentSudoku[tempRow*N + tempCol]->erroneousNeib.current->col != col) {
			currentSudoku[tempRow*N + tempCol]->erroneousNeib.current = currentSudoku[tempRow*N + tempCol]->erroneousNeib.current->next;
		}
		deleteNode(currentSudoku[tempRow*N + tempCol], &currentSudoku[tempRow*N + tempCol]->erroneousNeib, currentSudoku[tempRow*N + tempCol]->erroneousNeib.current);/*Delete the main node from the other lists */
		deleteNode(currentSudoku[row*N + col], &currentSudoku[tempRow*N + tempCol]->erroneousNeib, currentSudoku[row*N + col]->erroneousNeib.head); /*In order to Delete the main node's list*/
		if (currentSudoku[tempRow*N + tempCol]->erroneousNeib.head == NULL) {
			currentSudoku[tempRow*N + tempCol]->erroneous = 0;
		}
	}
}

void erroneousFixAdd(int row, int col, int val) {
	checkErrorRow(row, col, val);
	checkErrorCol(row, col, val);
	checkErrorBlock(row, col, val);
}

void set(Cell** currentSudoku, int row, int col, int val, char* oldCommand) {
	bool rowValid, colValid, blockValid;
	int oldVal;
	char* command;
	if (currentSudoku[row*N + col]->fixed == 1) {
		printf("Error: cell is fixed\n");
	}
	else {
		oldVal = currentSudoku[row*N + col]->value;
		addNode(&undo_redo, row, col, val, oldVal);
		if (val == 0) { /*we initialize the cell, change it to an empty cell*/
			currentSudoku[row*N + col]->value = 0;
			currentSudoku[row*N + col]->empty = 0;
			if (currentSudoku[row*N + col]->erroneous == 1) {
				erroneousFixDel(row, col, oldVal);
				currentSudoku[row*N + col]->erroneous = 0;
			}
			printSudoku(currentSudoku);
		}
		else { /*should delete #########################################################################################*/
			rowValid = isRowValidGame(currentSudoku, row, col, val);
			colValid = isColValidGame(currentSudoku, row, col, val);
			blockValid = isBlockValidGame(currentSudoku, row - row % blockWidth, col - col % blockHeight, row, col, val);
			if (rowValid && colValid && blockValid) {
				currentSudoku[row*N + col]->value = val;
				currentSudoku[row*N + col]->empty = 1; 
			currentSudoku[row*N + col]->value = val;
			currentSudoku[row*N + col]->empty = 1;
			if (currentSudoku[row*N + col]->erroneous == 1) {
				erroneousFixDel(row, col, oldVal);
				erroneousFixAdd(row, col, val);
			}
			else {
				erroneousFixAdd(row, col, val);
			}
				printSudoku(currentSudoku);
				if (isGameOver(currentSudoku)) { /*if true-from this moment we approve only 'restart' and 'exit' commands*/
					printf("Puzzle solved successfully\n");
					free(oldCommand);
					command = getCommand();
					while (command[0] != '4' && command[0] != '5') { /*'4'=restart, '5'=exit*/
						if (command[0] == '1' || command[0] == '2' || command[0] == '3') {
							printf("Error: invalid command\n");
						}
						free(command);
						command = getCommand();
					}
					if (command[0] == '4') {
						free(command);
						restart();
					}
					else {
						free(command);
						exitGame();
					}
				}
			}
			else {
				printf("Error: value is invalid\n");
			}
		}
	}
}

void exitGame() {
	freeSudoku(currentSudoku);
	freeSudoku(solvedSudoku);
	printf("Exiting...\n");
	exit(0);
}

void restart() {
	freeSudoku(currentSudoku);
	freeSudoku(solvedSudoku);
	puzzleGeneration(initNumberOfHints());
	playGame();
}

void solve(char* path) {
	char* fd;
	char buff[256] = "\0";
	char number[256] = "\0";
	int i = 0, dot = 0;
	int value, col, row;
	Cell** loadBoard;
	if ((fd = fopen(path, "r")) == NULL) {
		printf("Error: File doesn't exsist or cannot be opened\n");
		return;
	}
	if ((fscanf(fd, "%d %d", &col, &row)) != 2) {
		printf("Error: File doesn't exsist or cannot be opened\n");
		return;
	}
	blockHeight = row;
	blockWidth = col;
	N = blockWidth * blockHeight;
	loadBoard = (Cell**)malloc(sizeof(Cell*)*(col*col*row*row));
	for (int k = 0; k < (row*col); k++) {
		for (int j = 0; j < (col*row); j++) {
			fscanf(fd, "%s", buff);
			while (buff[i] != '\0') {
				if (buff[i] != '.') {
					number[i] = buff[i];
					buff[i] = '\0';
				}
				else if (buff[i] == '.') {
					dot = 1;
					buff[i] = '\0';
				}
				i++;
			}
			number[i] = '\0';
			i = 0;
			value = atoi(number);
			loadBoard[k*(col*row) + j] = createCell(value);
			if (value != 0) {
				loadBoard[k*(col*row) + j]->empty = 1;
			}
			if (dot == 1) {
				loadBoard[k*(col*row) + j]->fixed = 1;
			}
			dot = 0;
		}
	}
	mode = 1;
	//initList(undo_redo);
	currentSudoku = loadBoard;
}

void edit(char* path) {
	char* fd;
	char buff[256] = "\0";
	char number[256] = "\0";
	int i = 0, dot = 0;
	int value, col, row;
	Cell** loadBoard;
	if (*path == '\0') {
		loadBoard = generateSudoku();
		blockHeight = 3;
		blockWidth = 3;
		N = blockWidth * blockHeight;
	}
	else {
		if ((fd = fopen(path, "r")) == NULL) {
			printf("Error: File cannot be opened\n");
			return;
		}
		if ((fscanf(fd, "%d %d", &col, &row)) != 2) {
			printf("Error: File doesn't exsist or cannot be opened\n");
			return;
		}
		blockHeight = row;
		blockWidth = col;
		N = blockWidth * blockHeight;
		loadBoard = (Cell**)malloc(sizeof(Cell*)*(col*col*row*row));
		for (int k = 0; k < (row*col); k++) {
			for (int j = 0; j < (col*row); j++) {
				fscanf(fd, "%s", buff);
				while (buff[i] != '\0') {
					if (buff[i] != '.') {
						number[i] = buff[i];
						buff[i] = '\0';
					}
					else if (buff[i] == '.') {
						dot = 1;
						buff[i] = '\0';
					}
					i++;
				}
				number[i] = '\0';
				i = 0;
				value = atoi(number);
				loadBoard[k*(col*row) + j] = createCell(value);
				if (value != 0) {
					loadBoard[k*(col*row) + j]->empty = 1;
				}
				if (dot == 1) {
					loadBoard[k*(col*row) + j]->fixed = 1;
				}
				dot = 0;
			}
		}
	}
	mode = 2;
	markError = 1;
	//initList(undo_redo);
	currentSudoku = loadBoard;
}

void doCommand(char* command) {
	if (idCommand == 1) {
		solve(command);
	}
	else if (idCommand == 2) {
		edit(command);
	}
	else if (command[0] == '1') {
		set(currentSudoku, (command[2] - '0') - 1, (command[1] - '0') - 1, command[3] - '0', command);
	}
	else if (command[0] == '2') {
		hint((command[2] - '0') - 1, (command[1] - '0') - 1);
	}
	else if (command[0] == '3') {
		validate(currentSudoku);
	}

	else if (command[0] == '4') {
		free(command);
		restart();
	}
	else if (command[0] == '5') {
		free(command);
		exitGame();
	}
	else if (command[0] == '6') {
		markError = command[1] - '0';
	}
	else if (command[0] == '7') {
		printSudoku(currentSudoku);
	}
	free(command);
}

void getSolvedSudoku(Cell** boardGeneration) {
	solvedSudoku = boardGeneration;
}

void getSudokuWithHints(Cell** sudokuWithHints) {
	currentSudoku = sudokuWithHints;
}

void freeSudoku(Cell** sudoku) {
	int i, j;
	if (sudoku != NULL) {
		for (i = 0; i < N; i++) {
			for (j = 0; j < N; j++) {
				free(sudoku[i*N + j]);
			}
		}
		free(sudoku);
	}
}

void playGame() {
	char* command;
	while (true) {
		command = getCommand();
		doCommand(command);
	}
}

