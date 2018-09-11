#include<stdio.h>
#include<stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "Solver.h"
#include "Parser.h"
#include "Game.h"
#include "ILPAlgo.h"

int markError = 1; /*by default = 1*/
int undoBit; /*when we did undo to the first cell in the undo_redo list -> undoBit=1*/
int autoFillBit = 0; /*autoFillBit = 1 when the autofill command succeeds*/
int mode = 0; /*1 - solve mode and 2 - edit mode and 0 - init*/
int saveGlob = 0; /*when we do save command -> 1*/
int generateSudokuPrint = 0; /*when we don't want the sudoku to be printed in generate command*/
int generateGlobal = 0; /*1-> don't insert nodes to undo_redo list in generate command*/
/*
A linked list for all the play moves
*/
linkedList undo_redo;

/*
solvedSudoku is the solution of the board from the last time the user checked the validaty of the board
*/
Cell** solvedSudoku = NULL;

/*
currentSudoku is the board that the user is playing on
*/
Cell** currentSudoku = NULL;

int freeList(node* headNode) {
	node* temp;
	int countOfDelete = 0;
	while (headNode != NULL) {
		temp = headNode;
		headNode = headNode->next;
		free(temp);
		countOfDelete++;
	}
	return countOfDelete;
}

void initList(linkedList* l) {
	l->head = (node*)malloc(sizeof(node));
	l->head->prev = NULL;
	l->head->next = NULL;
	l->tail = l->head;
	l->current = l->tail;
	l->len = 0;
}

void addNode(linkedList* list, int row, int col, int val, int oldVal) {
	if (list->len == 0) {
		list->head->col = col;
		list->head->row = row;
		list->head->value = val;
		list->head->autoCells = 0;
		list->head->generateCells = 0;
		list->head->oldValue = oldVal;
		list->tail = list->head;
		list->current = list->tail;
	}
	else {
		list->tail->next = (node*)malloc(sizeof(node));
		list->tail->next->col = col;
		list->tail->next->row = row;
		list->tail->next->value = val;
		list->tail->next->oldValue = oldVal;
		list->tail->next->autoCells = 0;
		list->tail->next->generateCells = 0;
		list->tail->next->next = NULL;
		list->tail->next->prev = list->tail;
		list->tail = list->tail->next;
		list->current = list->tail;
	}
	list->len++;
}

void deleteErrorNeibNode(Cell* cell, linkedList* list, node* origNode) {
	if (list->len == 1) {
		free(origNode);
		initList(&cell->erroneousNeib);
	}
	else {
		if ((list->head->row == origNode->row) && (list->head->col == origNode->col)) { /*the head is origNode*/
			list->head = list->head->next;
			origNode->next->prev = origNode->prev;
			list->current = list->head;
		}
		else if (list->tail->row == origNode->row && list->tail->col == origNode->col) {
			list->tail = list->tail->prev;
			list->tail->next = origNode->next;
		}
		else {
			origNode->prev->next = origNode->next;
			origNode->next->prev = origNode->prev;
		}
		free(origNode);
		list->len--;
	}
}

void deleteListFrom(node* nodeToBeDeleted) {
	int numOfDeletedNodes = 0;
	if (nodeToBeDeleted->prev == NULL) {
		freeList(nodeToBeDeleted);
		undo_redo.len = 0;
	}
	else {
		nodeToBeDeleted->prev->next = NULL;
		numOfDeletedNodes = freeList(nodeToBeDeleted);
		undo_redo.len = undo_redo.len - numOfDeletedNodes;
	}
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
	initList(&cell->erroneousNeib);
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
	newCell->erroneousNeib = cell->erroneousNeib; /*################################copy one by one??*/
	return newCell;
}

void printSeparator() {
	int i = 0;
	for (i = 0; i < 4 * N + blockHeight + 1; i++) {
		printf("-");
	}
	printf("\n");
}

void printSudoku(Cell** sudoku) {
	int i, j, k, s;
	//printSeparator();



	for (i = 0; i < N; i++) {
		if (i % blockHeight == 0) {
			printSeparator();
		}
		for (j = 0; j < N; j++) {
			if (j % blockWidth == 0) {
				printf("|");
			}
			//currentNum = table[i][j].currentNum;
			printf(" ");
			if (sudoku[i*N + j]->empty == 0) {
				printf("  ");
			}
			else {
				printf("%2d", sudoku[i*N + j]->value);
			}
			if (currentSudoku[i*N + j]->fixed == 1) {
				printf(".");
			}
			else if (sudoku[i*N + j]->erroneous == 1 && (mode == 2 || markError == 1)) {
				printf("*");
			}
			else {
				printf(" ");
			}

		}
		printf("|\n");
	}
	printSeparator();


/*
	for (i = 0; i <blockWidth; i++) {
		for (j = 0; j < blockHeight; j++) {
			for (s = 0; s<blockHeight; s++) {
				printf("|");
				for (k = 0; k < blockWidth; k++) {
					printf(" ");
					if (sudoku[i*blockWidth*N + j * N + s * blockHeight + k]->empty == 0) {
						printf("   ");
					}
					else {
						printf("%2d", sudoku[i*blockWidth*N + j * N + s * blockHeight + k]->value);
						if (sudoku[i*blockWidth*N + j * N + s * blockHeight + k]->fixed == 1) {
							printf(".");
						}
						else if (sudoku[i*blockWidth*N + j * N + s * blockHeight + k]->erroneous == 1) {
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
	}*/
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
	for (i = 0; i < blockWidth; i++) {
		for (j = 0; j < blockHeight; j++) {
			if (sudoku[(i + startRow)*N + j + startCol]->value == num) {
				if (row != i + startRow || col != j + startCol) {
					return false;
				}
			}
		}
	}
	return true;
}

int validateForFinishGame(Cell** currentSudoku) {
	int ret = 0;
	if (isBoardErrorneus(currentSudoku) == 1)
	{
		return 2;
	}
	ret = ILPSolver();
	if (ret == 1) { /* board is solvable*/
		return 1;
	}
	else if (ret == 2) {
		return 2;
	}
	return 2;
}

int validate(Cell** currentSudoku) { /*return 0 - erroneous board, 1 - solvable, 2 - unsolvable*/
	int ret = 0;
	if (isBoardErrorneus(currentSudoku)==1)
	{
		printf("Error: board contains erroneous values\n");
		return 0;
	}
	ret = ILPSolver();
	if (ret == 1) { /* board is solvable*/
		if (saveGlob != 1) {
			printf("Validation passes: board is solvable\n");
		}
		return 1;
	}
	else if (ret == 2) {
		if (saveGlob != 1) {
			printf("Validation failed: board is unsolvable\n");
		}
		return 2;
	}
	return 2;
}

void hint(int row, int col) {
	int ret = 0;
	if (isBoardErrorneus(currentSudoku)==1) {
		printf("Error: board contains erroneous values\n");
		return;
	}
	if (currentSudoku[row*N + col]->fixed == 1) {
		printf("Error: cell is fixed\n");
		return;
	}
	if (currentSudoku[row*N + col]->empty == 1) {
		printf("Error: cell already contains a value\n");
		return;
	}
	ret = ILPSolver();
	if (ret == 2) {
		printf("Error: board is unsolvable\n");
		return;
	}
	else if (ret == 1) {
		printf("Hint: set cell to %d\n", solvedSudoku[row*N + col]->value);
	}
}

void checkErrorRow(int row, int col, int val) {
	int j = 0;
	for (j = 0; j < N; j++) {
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
	int j = 0;
	for (j = 0; j < N; j++) {
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
	int i, j, startRow, startCol;
	startRow = row - row % blockWidth;
	startCol = col - col % blockHeight;
	for (i = 0; i < blockWidth; i++) {
		for (j = 0; j < blockHeight; j++) {
			if (currentSudoku[(i + startRow)*N + j + startCol]->value == val) {
				if (row != i + startRow && col != j + startCol) {
					currentSudoku[row*N + col]->erroneous = 1;
					addNode(&currentSudoku[row*N + col]->erroneousNeib, i + startRow, j + startCol, val, 0);
					if (currentSudoku[(i + startRow)*N + j + startCol]->fixed == 0) {
						currentSudoku[(i + startRow)*N + j + startCol]->erroneous = 1;
						addNode(&currentSudoku[(i + startRow)*N + j + startCol]->erroneousNeib, row, col, val, 0);
					}
				}
			}
		}
	}
}

void erroneousFixAdd(int row, int col, int val) {
	checkErrorRow(row, col, val);
	checkErrorCol(row, col, val);
	checkErrorBlock(row, col, val);
}

void erroneousFixDel(int row, int col, int val) {
	int tempRow, tempCol;
	while (currentSudoku[row*N + col]->erroneousNeib.len != 0) {
		tempRow = currentSudoku[row*N + col]->erroneousNeib.head->row;
		tempCol = currentSudoku[row*N + col]->erroneousNeib.head->col;
		if (currentSudoku[tempRow*N + tempCol]->fixed == 1) {
			deleteErrorNeibNode(currentSudoku[row*N + col], &currentSudoku[row*N + col]->erroneousNeib, currentSudoku[row*N + col]->erroneousNeib.head); /*In order to Delete the main node's list*/
			continue;
		}
		currentSudoku[tempRow*N + tempCol]->erroneousNeib.current = currentSudoku[tempRow*N + tempCol]->erroneousNeib.head;
		while (currentSudoku[tempRow*N + tempCol]->erroneousNeib.current->row != row || /*Looking for the node to be deleted*/
			currentSudoku[tempRow*N + tempCol]->erroneousNeib.current->col != col) {
			currentSudoku[tempRow*N + tempCol]->erroneousNeib.current = currentSudoku[tempRow*N + tempCol]->erroneousNeib.current->next;
		}
		deleteErrorNeibNode(currentSudoku[tempRow*N + tempCol], &currentSudoku[tempRow*N + tempCol]->erroneousNeib, currentSudoku[tempRow*N + tempCol]->erroneousNeib.current);/*Delete the main node from the other lists */
		deleteErrorNeibNode(currentSudoku[row*N + col], &currentSudoku[row*N + col]->erroneousNeib, currentSudoku[row*N + col]->erroneousNeib.head); /*In order to Delete the main node's list*/
		currentSudoku[tempRow*N + tempCol]->erroneousNeib.current = currentSudoku[tempRow*N + tempCol]->erroneousNeib.head;
		if (currentSudoku[tempRow*N + tempCol]->erroneousNeib.len == 0) {
			currentSudoku[tempRow*N + tempCol]->erroneous = 0;
		}
	}
}

int isBoardErrorneus(Cell** sudoku){
	int i, j;
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			if (sudoku[i*N + j]->erroneous == 1)
			{
				return 1;
			}
		}
	}
	return 0;
}

int checkNumOfEmptyCells(Cell** sudoku){
	int count = 0, i, j;
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			if (sudoku[i* N + j]->empty == 0)
			{
				count++;
			}
		}
	}
	return count;
}

void set(Cell** sudoku, int row, int col, int val, char* oldCommand) {
	bool rowValid, colValid, blockValid;
	int oldVal, valid;
	char* command;
	if (sudoku[row*N + col]->fixed == 1) {
		printf("Error: cell is fixed\n");
	}
	else {
		oldVal = sudoku[row*N + col]->value;
		if (val == 0) { /*we initialize the cell, change it to an empty cell*/
			sudoku[row*N + col]->value = 0;
			sudoku[row*N + col]->empty = 0;
			if (sudoku[row*N + col]->erroneous == 1) {
				erroneousFixDel(row, col, oldVal);
				sudoku[row*N + col]->erroneous = 0;
			}
		}
		else {
			sudoku[row*N + col]->value = val;
			sudoku[row*N + col]->empty = 1;
			if (sudoku[row*N + col]->erroneous == 1) {
				erroneousFixDel(row, col, oldVal);
				sudoku[row*N + col]->erroneous = 0;
				erroneousFixAdd(row, col, val);
			}
			else {
				erroneousFixAdd(row, col, val);
			}
		}
		if (autoFillBit == 0 && generateSudokuPrint == 0) {
			printSudoku(sudoku);
		}
		if (undo_redo.len == 0 && generateGlobal == 0) {
			addNode(&undo_redo, row, col, val, oldVal);
		}
		else if (undoBit==1 && generateGlobal == 0) { /*we did undo to the first cell in undo_redo list*/
			deleteListFrom(undo_redo.current); /*deleteListFrom(X) - delete X and the nodes after until the end*/
			initList(&undo_redo);
			addNode(&undo_redo, row, col, val, oldVal);
			undoBit = 0;
		}
		else {
			if (generateGlobal == 0) {
				if (undo_redo.current->next == NULL) {
					addNode(&undo_redo, row, col, val, oldVal);
				}
				else {
					undo_redo.tail = undo_redo.current;
					deleteListFrom(undo_redo.current->next);
					addNode(&undo_redo, row, col, val, oldVal);
				}
			}
		}
		if (mode == 1 && checkNumOfEmptyCells(sudoku) == 0) { /*Last cell was filled*/
			valid = validateForFinishGame(sudoku);
			if (valid == 1) {
				printf("Puzzle solved successfully\n");
				mode = 0;
				return;
			}
			else printf("Puzzle solution erroneous\n");
		}
	}
}

void printAllChangesUndo(int* changesData) {
	int indx = 0;
	while (changesData[indx] != -1) {
		if (changesData[(indx + 2)] != 0 && changesData[(indx + 3)] != 0) {
			printf("Undo %d,%d: from %d to %d\n", (changesData[(indx + 1)] + 1), (changesData[indx] + 1), changesData[(indx + 2)], changesData[(indx + 3)]);
		}
		else if (changesData[(indx + 2)] == 0 && changesData[(indx + 3)] == 0) {
			printf("Undo %d,%d: from _ to _\n", (changesData[(indx + 1)] + 1), (changesData[indx] + 1));
		}
		else if (changesData[(indx + 2)] == 0) {
			printf("Undo %d,%d: from _ to %d\n", (changesData[(indx + 1)] + 1), (changesData[indx] + 1), changesData[(indx + 3)]);
			/*currentSudoku[changesData[indx] * N + changesData[(indx + 1)]]->empty = 1;*/
		}
		else {
			printf("Undo %d,%d: from %d to _\n", (changesData[(indx + 1)] + 1), (changesData[indx] + 1), changesData[(indx + 2)]);
			/*currentSudoku[changesData[indx] * N + changesData[(indx + 1)]]->empty = 0;*/
		}
		indx = indx + 4;
	}
}

void undoCurrent(int* changesData) {
	int row, col, beforeUndoVal, afterUndoVal;
	int indx = firstEmptyInArr(changesData);
	row= undo_redo.current->row;
	col = undo_redo.current->col;
	beforeUndoVal = undo_redo.current->value;
	afterUndoVal = undo_redo.current->oldValue;
	changesData[indx] = row;
	changesData[(indx+1)] = col;
	changesData[(indx + 2)] = beforeUndoVal;
	changesData[(indx + 3)] = afterUndoVal;
	if (undo_redo.current->prev != NULL) {
		undo_redo.current = undo_redo.current->prev;
	}
	else undoBit = 1;
	if (currentSudoku[row*N + col]->erroneous == 1) {
		erroneousFixDel(row, col, beforeUndoVal);
		currentSudoku[row*N + col]->erroneous = 0;
	}
	currentSudoku[row*N + col]->value = afterUndoVal;
	if (afterUndoVal != 0) {
		currentSudoku[row*N + col]->empty = 1;
		erroneousFixAdd(row, col, afterUndoVal);
	}
	else currentSudoku[row*N + col]->empty = 0;
}

void undo() {
	int numOfNodes = 0, j, k, i;
	int* changesData = (int*)malloc(sizeof(int)*((4 * N*N) + 4));
	node* temp = NULL;
	for (i = 0; i < N*N + 4; i++) {
		changesData[i] = -1;
	}
	if (undo_redo.len == 0 || undoBit == 1) {
		printf("Error: no moves to undo\n");
		return;
	}
	numOfNodes = undo_redo.current->autoCells;
	/*undo all relevant cells when we got to generate*/
	if (undo_redo.current->generateCells == 1) {
		while (undo_redo.current->generateCells == 1 && undoBit == 0) {
			undoCurrent(changesData);
		}
	}
	else if (numOfNodes > 0) {
		for (i = 0; i < numOfNodes-1; i++) {
			undo_redo.current = undo_redo.current->prev;
		}
		for (j = 0; j < numOfNodes; j++) {
			temp = undo_redo.current;
			undoCurrent(changesData);
			undo_redo.current = temp->next;
		}
		undo_redo.current = temp;
		for (k = 0; k < numOfNodes; k++) {
			if (undo_redo.current->prev == NULL) {
				undoBit = 1;
				break;
			}
			undo_redo.current = undo_redo.current->prev;
		}
	}
	else {
		undoCurrent(changesData);
	}
	printSudoku(currentSudoku);
	printAllChangesUndo(changesData);
	free(changesData);
}

int firstEmptyInArr(int* arr) {
	int i = 0;
	for (i; i < N*N; i++) {
		if (arr[i] == -1) {
			return i;
		}
	}
}

void PrintAllChangesRedo(int* changesData) {
	int indx = 0;
	while (changesData[indx] != -1) {
		if (changesData[(indx + 2)] != 0 && changesData[(indx + 3)] != 0) {
			printf("Redo %d,%d: from %d to %d\n", (changesData[(indx + 1)] + 1), (changesData[indx] + 1), changesData[(indx + 2)], changesData[(indx + 3)]);
		}
		else if (changesData[(indx + 2)] == 0 && changesData[(indx + 3)] == 0) {
			printf("Redo %d,%d: from _ to _\n", (changesData[(indx + 1)] + 1), (changesData[indx] + 1));
		}
		else if (changesData[(indx + 2)] == 0) {
			printf("Redo %d,%d: from _ to %d\n", (changesData[(indx + 1)] + 1), (changesData[indx] + 1), changesData[(indx + 3)]);
			currentSudoku[changesData[indx] *N + changesData[(indx + 1)]]->empty = 1;
		}
		else {
			printf("Redo %d,%d: from %d to _\n", (changesData[(indx + 1)] + 1), (changesData[indx] + 1), changesData[(indx + 2)]);
			currentSudoku[changesData[indx] *N + changesData[(indx + 1)]]->empty = 0;
		}
		indx = indx + 4;
	}
}

void redoCurrent(int row, int col, int beforeRedoVal, int afterRedoVal, int* changesData) {
	int indx = firstEmptyInArr(changesData);
	changesData[indx] = row;
	changesData[(indx + 1)] = col;
	changesData[(indx + 2)] = beforeRedoVal;
	changesData[(indx + 3)] = afterRedoVal;
	if (undoBit == 1) {
		undoBit = 0;
	}
	else if (undo_redo.current->next != NULL) {
		undo_redo.current = undo_redo.current->next;
	}
	if (currentSudoku[row*N + col]->erroneous == 1) {
		erroneousFixDel(row, col, beforeRedoVal);
		currentSudoku[row*N + col]->erroneous = 0;
	}
	currentSudoku[row*N + col]->value = afterRedoVal;
	if (afterRedoVal != 0) {
		currentSudoku[row*N + col]->empty = 1;
		erroneousFixAdd(row, col, afterRedoVal);
	}
	else currentSudoku[row*N + col]->empty = 0;
}

void redo() {
	int col, row, beforeRedoVal, afterRedoVal, i = 0;
	int* changesData = (int*)malloc(sizeof(int) * ((4 * N * N) + 4));
	for (i = 0; i < N*N + 4; i++) {
		changesData[i] = -1;
	}
	if (undo_redo.len == 0 || (undo_redo.current->next == NULL && undoBit == 0)) {
		printf("Error: no moves to redo\n");
		return;
	}
	if (undo_redo.current->next == NULL && undoBit == 1) {
		col = undo_redo.current->col;
		row = undo_redo.current->row;
		beforeRedoVal = undo_redo.current->oldValue;
		afterRedoVal = undo_redo.current->value;
		redoCurrent(row, col, beforeRedoVal, afterRedoVal, changesData);
	}
	else if (undo_redo.current->generateCells == 1 && undoBit == 1) {
		col = undo_redo.current->col;
		row = undo_redo.current->row;
		beforeRedoVal = undo_redo.current->oldValue;
		afterRedoVal = undo_redo.current->value;
		redoCurrent(row, col, beforeRedoVal, afterRedoVal, changesData);
		while (undo_redo.current->next != NULL && undo_redo.current->next->generateCells == 1) {
			col = undo_redo.current->next->col;
			row = undo_redo.current->next->row;
			beforeRedoVal = undo_redo.current->next->oldValue;
			afterRedoVal = undo_redo.current->next->value;
			redoCurrent(row, col, beforeRedoVal, afterRedoVal, changesData);
		}
	}

/*
	else if (undo_redo.current->generateCells == 1 ) {
		printf("in redo - generatecells =1\n");
		if (undoBit == 1) {
			col = undo_redo.current->col;
			row = undo_redo.current->row;
			beforeRedoVal = undo_redo.current->oldValue;
			afterRedoVal = undo_redo.current->value;
			redoCurrent(row, col, beforeRedoVal, afterRedoVal);
		}
		do {
			col = undo_redo.current->next->col;
			row = undo_redo.current->next->row;
			beforeRedoVal = undo_redo.current->next->oldValue;
			afterRedoVal = undo_redo.current->next->value;
			redoCurrent(row, col, beforeRedoVal, afterRedoVal);
		} while (undo_redo.current->generateCells == 1 && undo_redo.current->next != NULL);
	}
	*/

	/*while ((undo_redo.current->generateCells == 1 && (undo_redo.current->next == NULL && undoBit == 1)) || (undo_redo.current->generateCells == 1 && undo_redo.current->next != NULL)) {
		col = undo_redo.current->col;
		row = undo_redo.current->row;
		beforeRedoVal = undo_redo.current->oldValue;
		afterRedoVal = undo_redo.current->value;
		redoCurrent(row, col, beforeRedoVal, afterRedoVal);
	}
	*/
	
	else if (undo_redo.current->next->autoCells == 0) {
		col = undo_redo.current->next->col;
		row = undo_redo.current->next->row;
		beforeRedoVal = undo_redo.current->next->oldValue;
		afterRedoVal = undo_redo.current->next->value;
		redoCurrent(row, col, beforeRedoVal, afterRedoVal, changesData);
	}
	else {
		if (undoBit == 1 && undo_redo.current->autoCells == 1) {
			col = undo_redo.current->col;
			row = undo_redo.current->row;
			beforeRedoVal = undo_redo.current->oldValue;
			afterRedoVal = undo_redo.current->value;
			redoCurrent(row, col, beforeRedoVal, afterRedoVal, changesData);
			undo_redo.current = undo_redo.current->prev;
			undoBit = 0;
		}
		while ((undo_redo.current->next != NULL) && (undo_redo.current->next->autoCells - 1 == undo_redo.current->autoCells)) {
			col = undo_redo.current->next->col;
			row = undo_redo.current->next->row;
			beforeRedoVal = undo_redo.current->next->oldValue;
			afterRedoVal = undo_redo.current->next->value;
			redoCurrent(row, col, beforeRedoVal, afterRedoVal, changesData);
		}
	}
	printSudoku(currentSudoku);
	PrintAllChangesRedo(changesData);
	free(changesData);
}

void exitGame() {
	freeSudoku(currentSudoku);
	freeSudoku(solvedSudoku);
	freeList(undo_redo.head);
	printf("Exiting...\n");
	exit(0);
}

void reset() {
	int i = 0;
	for (i = 0; i < undo_redo.len; i++) {
		if (undoBit == 1) {
			break;
		}
		undo();
	}
	freeList(undo_redo.head);
	initList(&undo_redo);
	printf("Board reset\n");
}

void save(Cell** sudoku, char* path) { /*we should check if it is possible to save an erroneus board in solve mode,
									   i think its possilbe. If its possible, we need to check after loading the board in solve
									   and edit if there is and erroneus cells*/
	int k, j;
	if (mode == 2) {
		if (isBoardErrorneus(sudoku) == 1) {
			printf("Error: board contains errorneus values\n");
			return;
		}
		saveGlob = 1;
		if (validate(currentSudoku) == 2) {
			printf("Error: board validation failed\n");
			saveGlob = 0;
			return;
		}
		saveGlob = 0;
	}
	FILE* fd = fopen(path, "w");
	if (fd == NULL) {
		printf("Error: File cannot be created or modified\n");
	}
	fprintf(fd, "%c%c%c", (blockHeight+'0'),' ', (blockWidth+'0'));
	for (k = 0; k < (blockHeight*blockWidth); k++) {
				fprintf(fd,"\n");
		for (j = 0; j < (blockHeight*blockWidth); j++) {
			if (mode == 2) {
				if (currentSudoku[k*(blockHeight*blockWidth) + j]->empty == 0) {
					fprintf(fd, "%c%c", (currentSudoku[k*(blockHeight*blockWidth) + j]->value + '0'), ' ');
				}
				else {
					fprintf(fd, "%c%c%c", (currentSudoku[k*(blockHeight*blockWidth) + j]->value + '0'), '.', ' ');
				}
			}
			else {
				if (currentSudoku[k*(blockHeight*blockWidth) + j]->fixed == 1) {
					fprintf(fd, "%c%c%c", (currentSudoku[k*(blockHeight*blockWidth) + j]->value + '0'), '.', ' ');
				}
				else fprintf(fd, "%c%c", (currentSudoku[k*(blockHeight*blockWidth) + j]->value + '0'), ' ');
			}
		}
	}
	fclose(fd);
	printf("Saved to: %s\n", path);
}

void autoFill(Cell** sudoku) {
	int k, j, z, g;
	if (isBoardErrorneus(sudoku) == 1) {
		printf("Error: board contains erroneous values\n");
		return;
	}
	int* arr = (int*)malloc(N * sizeof(int)*3);
	int numCounter = 0; /*for counting the number of the possible fills*/
	int counter = 0; /*for counting the number of cell to autofill*/
	int startRow, startCol, tempRow, tempCol, tempVal;
	for (k = 0; k < N; k++) {
		for (j = 0; j < N; j++) {
			if (currentSudoku[k*N + j]->empty == 0) {/*check only the empty cells*/
				startRow = k - k % blockWidth;
				startCol = j - j % blockHeight;
				for (z = 1; z <= N; z++) {/*for every empty cell in the sudoku we check which
																	number between 1-N is valid for him. if there is more of
																	1 option, numCounter wil be > 1*/
					if (isRowValidGame(sudoku, k, j, z) && isColValidGame(sudoku, k, j, z) && isBlockValidGame(sudoku,startRow,startCol, k, j, z)) {
						tempRow = k; tempCol = j; tempVal = z;
						numCounter++;
					}
				}
				if (numCounter == 1) {/*there is only 1 option between 1-N
									  we search for the option and add to the list*/
					arr[counter] = tempRow; arr[counter + 1] = tempCol; arr[counter + 2] = tempVal;
					counter = counter + 3;
				}
			}
			numCounter = 0;
		}
	}
	for (g = 0; g < counter; g=g+3) {
		autoFillBit = 1;
		printf("Cell <%d,%d> set to %d\n", arr[g + 1] + 1, arr[g] + 1, arr[g + 2]);
		set(sudoku, arr[g], arr[g + 1], arr[g + 2], NULL);
		undo_redo.tail->autoCells = (g/3)+1;
	}
	printSudoku(sudoku);
	autoFillBit = 0;
	free(arr);
}

void num_solutions(Cell** sudoku) {
	int i, j, num;
	if (isBoardErrorneus(sudoku) == 1) {
		printf("Error: board contains erroneous values\n");
		return;
	}
	Cell** temp = (Cell**)malloc(sizeof(Cell*)*(N*N));
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			temp[i*N + j] = createCell(currentSudoku[i*N + j]->value);
			if (temp[i*N + j]->value != 0) {
				temp[i*N + j]->fixed = 1;
				temp[i*N + j]->empty = 1;
			}
		}
	}
	num = exBackTrack(temp);
	printf("Number of solutions: %d\n", num);
	if (num == 1) {
		printf("This is a good board!\n");
	}
	else printf("The puzzle has more than 1 solution, try to edit it further\n");
	freeSudoku(temp);
	return;
}

void checkErroneous(Cell** sudoku) {
	int i, j;
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			if (sudoku[i*N + j]->value != 0) {
				erroneousFixAdd(i, j, sudoku[i*N + j]->value);
			}
		}
	}
}

void solve(char* path) {
	FILE* fd;
	char buff[256] = "\0";
	char number[256] = "\0";
	int i = 0, dot = 0, k = 0, j;
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
	if (currentSudoku != NULL) {/* free the memory allocations from the prev game*/
		freeList(undo_redo.head);
		freeSudoku(currentSudoku);
		if (solvedSudoku) {
			freeSudoku(solvedSudoku);
		}
	}
	blockWidth = row; /*n*/
	blockHeight = col; /*m*/
	N = blockHeight * blockWidth;
	loadBoard = (Cell**)malloc(sizeof(Cell*)*(col*col*row*row));
	for (k = 0; k < (row*col); k++) {
		for (j = 0; j < (col*row); j++) {
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
	initList(&undo_redo);
	currentSudoku = loadBoard;
	checkErroneous(currentSudoku);
	printSudoku(currentSudoku);
}

Cell** generateSudokuGame() {
	int i, j;
	N = 9;
	Cell** sudoku = (Cell**)malloc(sizeof(Cell*)*N*N);
	if (sudoku == NULL) {
		printf("Error: generateSudoku has failed\n");
		exit(0);
	}
	for (i = 0; i < N; i++) { /*creating an empty sudoku*/
		for (j = 0; j < N; j++) {
			sudoku[i* N + j] = createCell(0);
		}
	}
	return sudoku;
}

void edit(char* path) {
	FILE* fd;
	char buff[256] = "\0";
	char number[256] = "\0";
	int i = 0, dot = 0, k = 0,j, value, col, row;
	Cell** loadBoard;

	if (*path == '\0') {
		if (currentSudoku != NULL) { /* free the memory allocations from the prev game*/
			freeList(undo_redo.head);
			freeSudoku(currentSudoku);
			if (solvedSudoku) {
				freeSudoku(solvedSudoku);
			}
		}
		loadBoard = generateSudokuGame();
		blockWidth = 3;
		blockHeight = 3;
		N = blockHeight * blockWidth;
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
		if (currentSudoku != NULL) { /* free the memory allocations from the prev game*/
			freeList(undo_redo.head);
			freeSudoku(currentSudoku);
			if (solvedSudoku) {
				freeSudoku(solvedSudoku);
			}
		}
		blockWidth = row;
		blockHeight = col;
		N = blockHeight * blockWidth;
		loadBoard = (Cell**)malloc(sizeof(Cell*)*(col*col*row*row));
		for (k = 0; k < (row*col); k++) {
			for (j = 0; j < (col*row); j++) {
				fscanf(fd, "%s", buff);
				while (buff[i] != '\0') {
					if (buff[i] != '.') {
						number[i] = buff[i];
						buff[i] = '\0';
					}/*
					else if (buff[i] == '.') {
						dot = 1;
						buff[i] = '\0';
					}*/
					i++;
				}
				number[i] = '\0';
				i = 0;
				value = atoi(number);
				loadBoard[k*(col*row) + j] = createCell(value);
				if (value != 0) {
					loadBoard[k*(col*row) + j]->empty = 1;
				}/*
				if (dot == 1) {
					loadBoard[k*(col*row) + j]->fixed = 1;
				}*/
				dot = 0;
			}
		}
	}
	mode = 2;
	markError = 1;
	initList(&undo_redo);
	currentSudoku = loadBoard;
	checkErroneous(currentSudoku);
	printSudoku(currentSudoku);
}

void clearBoard(Cell** sudoku) {
	int i = 0, j = 0;
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			sudoku[i*N + j]->empty = 0;
			sudoku[i*N + j]->value = 0;
		}
	}
}

/*should check the result!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

void generate(int x, int y) {
	int i, row, col, val, ret, countNum = 0, iter = 0, j, k = 0;
	int startRow, startCol;
	int numOfEmptyCells = checkNumOfEmptyCells(currentSudoku);
	if (numOfEmptyCells != (N*N)) {
		printf("Error: board is not empty\n");
		return;
	}

	while (iter <= 1000){
		for (i = 0; i < x; i++) {
			row = rand() % N;
			col = rand() % N;
			while (currentSudoku[row*N + col]->empty == 1) {
				row = rand() % N;
				col = rand() % N;
			}
			countNum = 0;
			startRow = row - row % blockWidth;
			startCol = col - col % blockHeight;
			for (j = 1; j <= N; j++) {
				if (isRowValidGame(currentSudoku, row, col, j) && isColValidGame(currentSudoku, row, col, j) && isBlockValidGame(currentSudoku, startRow, startCol, row, col, j)) {
					countNum++;
				}
			}
				if (countNum == 0) {
					iter++;
					countNum = 0;
					break;
				}
				val = rand() % N;
				startRow = row - row % blockWidth;
				startCol = col - col % blockHeight;
				while (!(isRowValidGame(currentSudoku, row, col, val) && isColValidGame(currentSudoku, row, col, val) && isBlockValidGame(currentSudoku, startRow, startCol, row, col, val))) {
					val = rand() % N;
				}
				generateGlobal = 1;
				generateSudokuPrint = 1;
				set(currentSudoku, row, col, val, NULL); /* to insert the new cell to the sudoku */
				generateGlobal = 0;
				generateSudokuPrint = 0;
			}
		ret = ILPSolver();
		if (ret != 1) { /* we didn't get a solution from the gurobi */
			iter++;
		}
		else {
			break;
		}
		clearBoard(currentSudoku);
	}
	if (iter > 1000) {
		printf("Error: puzzle generator failed\n");
		clearBoard(currentSudoku);
		return;
	}
	clearBoard(currentSudoku);
	for (k = 0; k < y; k++) {
		row = rand() % N;
		col = rand() % N;
		while (currentSudoku[row*N + col]->empty == 1) {
			row = rand() % N;
			col = rand() % N;
		}
		if (currentSudoku[row*N + col]->empty == 0) {
			generateSudokuPrint = 1;
			set(currentSudoku, row, col, solvedSudoku[row*N + col]->value, NULL);
			generateSudokuPrint = 0;
			undo_redo.tail->generateCells = 1;
			/*currentSudoku[row*N + col]->empty = 1;
			currentSudoku[row*N + col]->value = solvedSudoku[row*N + col]->value;*/
		}
	}
	printSudoku(currentSudoku);
	return;
}

void doCommand(char* command) {
	if (idCommand == 1) {
		solve(command);
	}
	else if (idCommand == 2) {
		edit(command);
	}
	else if (idCommand == 3) {
		save(currentSudoku, command);
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
		reset();
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
	else if (command[0] == '8') {
		autoFill(currentSudoku);
	}
	else if (command[0] == '9') {
		num_solutions(currentSudoku);
	}
	else if (command[0] == 'a') {
		redo();
	}
	else if (command[0] == 'b') {
		undo();
	}
	else if (command[0] == 'c') { /*generate*/
		generate(command[1] - '0', command[2] - '0');
	}
	free(command);
}

void getSudokuWithHints(Cell** sudokuWithHints) {
	currentSudoku = sudokuWithHints;
}

void freeSudoku(Cell** sudoku) {
	int i, j;
	if (sudoku != NULL) {
		for (i = 0; i < N; i++) {
			for (j = 0; j < N; j++) {
				if (sudoku[i*N + j]->fixed == 0) {
					freeList(sudoku[i*N + j]->erroneousNeib.head);
				}
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

