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
int solvedInSet = 0; /*1 -> we did solve and the puzzle has successfully solved*/
int resetPrint = 0; /*when we don't want the sudoku to be printed in reset command*/
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

/*
This function gets a node in the doubly linked list and free all the memory allocations starts by this node
(free 'headNode' and all the nodes after it in the doubly linked list)
@return
the number of cells the function did free to
*/
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

/*
This function initiallizes the doubly linked list:
allocates space in memory for the 'head' field
init head.prev and head.next as NULL
init the len field to 0
define tail and current to be the head
*/
void initList(linkedList* l) {
	l->head = (node*)malloc(sizeof(node));
	l->head->prev = NULL;
	l->head->next = NULL;
	l->tail = l->head;
	l->current = l->tail;
	l->len = 0;
}

/*
This function adds a node to the end of a linked list,
it gets a doubly linked list and the fields of the new node:
allocates memory space for the node if needed,
updates the tail and current to the new node,
update the len field
*/
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

/*
This function deletes a specific node from the erroneousNeib doubly linked list
it gets the cell of the node to be deleted, a doubly linked list and the node to be deleted:
if origNode is the only node in the linked list, it frees the node and creates a new erroneousNeib list
else it updates the list and its len and frees origNode
*/
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

/*
This function deletes all the nodes from 'nodeToBeDeleted' to the tail in the doubly linked list:
if nodeToBeDeleted is the head it frees the entire list by using 'freeList' function and updates the len to 0
else, it updates the last node to be NULL and deletes the node from nodeToBeDeleted, and updates the len
*/
void deleteListFrom(node* nodeToBeDeleted) {
	int numOfDeletedNodes = 0;
	if (nodeToBeDeleted->prev == NULL) {/*nodeToBeDeleted=head*/
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

/*
This function get a cell and copy his fields
@return
copy of the cell
*/
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

/*
This function prints the separators between the blocks of the sudoku
*/
void printSeparator() {
	int i = 0;
	for (i = 0; i < 4 * N + blockHeight + 1; i++) {
		printf("-");
	}
	printf("\n");
}

/*
This function prints the sudoku boards as instructed using printSeparator func
*/
void printSudoku(Cell** sudoku) {
	int i, j;
	for (i = 0; i < N; i++) {
		if (i % blockHeight == 0) {
			printSeparator();
		}
		for (j = 0; j < N; j++) {
			if (j % blockWidth == 0) {
				printf("|");
			}
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

/*
This function gets a complete sudoku and checks wether it valid or not using ILP algorithm.
*/
int validateForFinishGame(Cell** currentSudoku) {
	int ret = 0;
	if (isBoardErroneous(currentSudoku) == 1)
	{
		return 2;
	}
	ret = ILPSolver();
	if (ret == 1) { /* board is solvable*/
		freeSudoku(solvedSudoku);/*from ILPSolver*/
		return 1;
	}
	else if (ret == 2) {
		return 2;
	}
	return 2;
}

/*
This function gets the current game Sudoku board and checks if there is any solution of this board
by using ILP in module 'ILPAlgo'
@return
0 -> the board contains erroneous values
1 -> the board is solvable (we found a solution using ILP)
2 -> the board is unsolvable or the ILP algorithm didn't work
*/
int validate(Cell** currentSudoku) { /*return 0 - erroneous board, 1 - solvable, 2 - unsolvable*/
	int ret = 0;
	if (isBoardErroneous(currentSudoku)==1)
	{
		printf("Error: board contains erroneous values\n");
		return 0;
	}
	ret = ILPSolver();
	if (ret == 1) { /* board is solvable*/
		if (saveGlob != 1) {
			printf("Validation passed: board is solvable\n");
		}
		freeSudoku(solvedSudoku);/*from ILPSOlver*/
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

/*
This function gets a row and a column, solve the current coard by using ILP,
and print the value of the cell that the ILP found.
@return
nothing -> when the currentsudoku is erroneous or the cell is fixed or the cell is not empty
or the board is unsolvable
solvedSudoku[row][column] that saved the solution of the current board -> if the board is solvable
*/
void hint(int row, int col) {
	int ret = 0;
	if (isBoardErroneous(currentSudoku)==1) {
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
	else if (ret == 1) {/*the board is solvable*/
		printf("Hint: set cell to %d\n", solvedSudoku[row*N + col]->value);
		freeSudoku(solvedSudoku);/*from ILPSolver*/
	}
}

/*
This function handles the erroneous cells in a given row
it gets a row, col and a value:
it goes over the columns of the sudoku and checks for each cell in the given row if it's value equals val,
if so- checks wether j!=col (if its not the given cell),
if so- these cells are erroneous,
it adds the cell it found to the erroneousNeib list of the given cell,
and adds the given cell to the erroneousNeib list of the cell it found
in addition it updates the erroneous fiels of the cells to 1
*/
void checkErrorRow(int row, int col, int val) {
	int j = 0;
	for (j = 0; j < N; j++) {
		if (currentSudoku[(row)*N + j]->value == val) {
			if (j != col) {
				currentSudoku[row*N + col]->erroneous = 1;
				addNode(&currentSudoku[row*N + col]->erroneousNeib, row, j, val, 0);/*add the cell it found to the current cell's list*/
				if (currentSudoku[row*N + j]->fixed == 0) {
					currentSudoku[row*N + j]->erroneous = 1;
					addNode(&currentSudoku[row*N + j]->erroneousNeib, row, col, val, 0);/*add the current cell to the list of the cell we found*/
				}
			}
		}
	}
}

/*
This function handles the erroneous cells in a given column
it gets a row, col and a value:
it goes over the rows of the sudoku and checks for each cell in the given column if it's value equals val,
if so- checks wether j!=row (if its not the given cell),
if so- these cells are erroneous,
it adds the cell it found to the erroneousNeib list of the given cell,
and adds the given cell to the erroneousNeib list of the cell it found
in addition it updates the erroneous fiels of the cells to 1
*/
void checkErrorCol(int row, int col, int val) {
	int j = 0;
	for (j = 0; j < N; j++) {
		if (currentSudoku[j*N + col]->value == val) {
			if (j != row) {
				currentSudoku[row*N + col]->erroneous = 1;
				addNode(&currentSudoku[row*N + col]->erroneousNeib, j, col, val, 0);/*add the cell it found to the current cell's list*/
				if (currentSudoku[j*N + col]->fixed == 0) {
					currentSudoku[j*N + col]->erroneous = 1;
					addNode(&currentSudoku[j*N + col]->erroneousNeib, row, col, val, 0);/*add the current cell to the list of the cell we found*/
				}
			}
		}
	}
}

/*
This function handles the erroneous cells in a given block
it gets a row, col and a value:
startRow= the index of the first row of the block
startCol= the index of the first column in the block
it goes over the rows of the block and over the columns of the block,
and checks for each cell in the given block if it's value equals val,
if so-> checks wether the cell it found is not at the same row and column (if it is in the same row for example, we found it already in 'checkReeoeRow' func),
if so-> these cells are erroneous,
it adds the cell it found to the erroneousNeib list of the given cell,
and adds the given cell to the erroneousNeib list of the cell it found
in addition it updates the erroneous fiels of the cells to 1
*/
void checkErrorBlock(int row, int col, int val) {
	int i, j, startRow, startCol;
	startRow = row - row % blockHeight;
	startCol = col - col % blockWidth;
	for (i = 0; i < blockHeight; i++) {
		for (j = 0; j < blockWidth; j++) {
			if (currentSudoku[(i + startRow)*N + j + startCol]->value == val) {
				if (row != i + startRow && col != j + startCol) {
					currentSudoku[row*N + col]->erroneous = 1;
					addNode(&currentSudoku[row*N + col]->erroneousNeib, i + startRow, j + startCol, val, 0);/*add the cell it found to the current cell's list*/
					if (currentSudoku[(i + startRow)*N + j + startCol]->fixed == 0) {
						currentSudoku[(i + startRow)*N + j + startCol]->erroneous = 1;
						addNode(&currentSudoku[(i + startRow)*N + j + startCol]->erroneousNeib, row, col, val, 0);/*add the current cell to the list of the cell we found*/
					}
				}
			}
		}
	}
}

/*
This function handles all the erroneous cells after inserting a new value to the sudoku:
it gets the row, col and value of the new cell in the sudoku
it uses 'checkErrorRow', 'checkErrorCol' and 'checkErrorBlock' functions.
*/
void erroneousFixAdd(int row, int col, int val) {
	checkErrorRow(row, col, val);
	checkErrorCol(row, col, val);
	checkErrorBlock(row, col, val);
}

/*
This function handles all the erroneous cells after deleting value from the sudoku:
it gets the row, col and value of the deleted cell
the function goes over the erroneousNieb list of the deleted cell, and for every cell C in that list:
1. delete C's node from the deleted cell's list
2. if the C is not fixed:
2.1 delete the deleted cell's node from the C's list
2.2 update erroneousNieb len of C and if needed update it's erroneous field (if len=0)
*/
void erroneousFixDel(int row, int col, int val) {
	int tempRow, tempCol;
	while (currentSudoku[row*N + col]->erroneousNeib.len != 0) {/*there are more arroneous cells to be fixed*/
		tempRow = currentSudoku[row*N + col]->erroneousNeib.head->row;
		tempCol = currentSudoku[row*N + col]->erroneousNeib.head->col;
		if (currentSudoku[tempRow*N + tempCol]->fixed == 1) {/*delete only from the current cell's list*/
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

/*
This function checks if there is an erroneous cell in the sudoku:
it gets a sudoku,
it goes over all the cells in the sudoku and checks it's erroneous field
@return
1 if erroneous, 0 if not erroneous
*/
int isBoardErroneous(Cell** sudoku){
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

/*
This function counts the number of empty cells in the sudoku:
it gets a sudoku,
it goes over all the cells in the sudoku and checks it's empty field,
if the cell is empty - adds the counter 1
@return
the counter
*/
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

/*
This function gets sudoku, row, column, value and old command.
it checks if it is valid to put the value in currentSudoku[row][column],
if so it inserts the value to the cell, update it's erroneousNeib list by
using 'erroneousFixDel' and 'erroneousFixAdd' functions.
in addition it adds a node to the undo_redo list by using 'addNode' function,
and delete all nodes after the current pointer by using 'deleteListFrom' function.
^^if undoBit==1 -> the last move the player did before was undo to the first node
in undo_redo list.
^^if mode==1 and the number of empty cells==0 -> the game complete.
*/
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
			else {/*when the cell wasn't erroneous before - don't need to use fixDel*/
				erroneousFixAdd(row, col, val);
			}
		}
		if (autoFillBit == 0 && generateSudokuPrint == 0) {/*when autofill or generate commands uses set function- we don't want to print the board here*/
			printSudoku(sudoku);
		}
		if (undo_redo.len == 0 && generateGlobal == 0) {/*when we are not in generate command*/
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
				if (autoFillBit == 1) {
					printSudoku(currentSudoku);
				}
				printf("Puzzle solved successfully\n");
				mode = 0;
				solvedInSet = 1;
				return;
			}
			else printf("Puzzle solution erroneous\n");
		}
	}
}

/*
This function gets an int array - changesData which contains all the changes that have made in the undo command.
it prints all the changes one by one as required.
*/
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
		}
		else {
			printf("Undo %d,%d: from %d to _\n", (changesData[(indx + 1)] + 1), (changesData[indx] + 1), changesData[(indx + 2)]);
		}
		indx = indx + 4;
	}
}

/*
This function makes undo to the current pointer of the undo_redo list:
it checks what is the current cell parameters (col, row, before undo value, after undo value),
changes the value of that cell to the after undo value,
and prints the the undo move.
it changes the current pointer to be the prev node.
^^if the prev node is NULL (we made undo to the first node) -> undoBit=1.
it fixes the cells' erroneousNeib lists by using 'erroneousFixDel' and 'erroneousFixAdd' functions.
*/
void undoCurrent(int* changesData) {
	int row, col, beforeUndoVal, afterUndoVal;
	int indx = firstEmptyInArr(changesData);
	row= undo_redo.current->row;
	col = undo_redo.current->col;
	beforeUndoVal = undo_redo.current->value;
	afterUndoVal = undo_redo.current->oldValue;
	changesData[indx] = row; /*saving the undo move to the int array*/
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

/*
This function executes the 'undo' command:
if undo_redo len==0 -> there are no moves to undo.
if the autoCells of the current pointer of the undo_redo list > 0 -> it means that we do undo to autofill:
we move the current pointer to be the prev node for autocCells-1 times,
then we do undo to the current pointer by using 'udoCurrent' function, and change the current pointer to the next node,
then we change the current pointer again to it's prev for autoCells times.
else -> we have only one node to undo, and we execute it by 'undoCurrent' function
*/
void undo() {
	int numOfNodes = 0, j, k, i;
	int* changesData = (int*)malloc(sizeof(int)*((4 * N*N) + 4));
	node* temp = NULL;
	for (i = 0; i < N*N + 4; i++) {/*initialize the int array for all the undo moves we want to print*/
		changesData[i] = -1;
	}
	if (undo_redo.len == 0 || undoBit == 1) {/*undoBit==1 -> we did undo command for the first node*/
		printf("Error: no moves to undo\n");
		free(changesData);
		return;
	}
	numOfNodes = undo_redo.current->autoCells;
	if (undo_redo.current->generateCells == 1) {
		while (undo_redo.current->generateCells == 1 && undoBit == 0) {/*undo all relevant cells when we do undo to generate command*/
			undoCurrent(changesData);
		}
	}
	else if (numOfNodes > 0) {/*undo all the relevent cells when we do undo to autofill command*/
		for (i = 0; i < numOfNodes-1; i++) {/*go to the first node that relates to the current autofill*/
			undo_redo.current = undo_redo.current->prev;
		}
		for (j = 0; j < numOfNodes; j++) {/*unddo the current node and move to the next one*/
			temp = undo_redo.current;
			undoCurrent(changesData);
			undo_redo.current = temp->next;
		}
		undo_redo.current = temp;
		for (k = 0; k < numOfNodes; k++) {/*updates the current pointer to be prev the autofill nodes*/
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
	if (resetPrint == 0) {/*when we do undo from reset command we don't want to print that*/
		printSudoku(currentSudoku);
		printAllChangesUndo(changesData);
	}
	free(changesData);
}

/*
This function gets an int array-arr and finds the first empty cell in arr
@return
the index of the first empty cell
*/
int firstEmptyInArr(int* arr) {
	int i = 0;
	for (i; i < N*N; i++) {
		if (arr[i] == -1) {
			return i;
		}
	}
}

/*
This function gets an int array - changesData which contains all the changes that have made in the redo command.
it prints all the changes one by one as required.
*/
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

/*
This function executes redo for the current pointer in undo_redo list.
it gets a row, col, before redo value and after redo value.
it prints the redo move as asked,
it changes the current pointer to be the next node:
^^if undoBit==1 and the next node is NULL -> we don't change the current pointer.
else if we have a next node -> we change it.
afterwards we fix the cells' erroneousNeib lists by using 'erroneousFixDel' and 'erroneousFixAdd' functions.
*/
void redoCurrent(int row, int col, int beforeRedoVal, int afterRedoVal, int* changesData) {
	int indx = firstEmptyInArr(changesData);
	changesData[indx] = row;/*saves the redo move that we want to print to the array*/
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

/*
This function do the redo command:
it checks if there are no moves to redo.
then it checks which of the possibilities is the relevent and use redoCurrent as many times as needed.
in the end it prints the board when needed and prints all the changes it made using PrintAllChangesRedo function. 
*/
void redo() {
	int col, row, beforeRedoVal, afterRedoVal, i = 0;
	int* changesData = (int*)malloc(sizeof(int) * ((4 * N * N) + 4));
	for (i = 0; i < N*N + 4; i++) {/*initialize the array of the redo moves to be printed later*/
		changesData[i] = -1;
	}
	if (undo_redo.len == 0 || (undo_redo.current->next == NULL && undoBit == 0)) {
		printf("Error: no moves to redo\n");
		free(changesData);
		return;
	}
	if (undo_redo.current->next == NULL && undoBit == 1) {/*one node in the list and we did undo to it before*/
		col = undo_redo.current->col;
		row = undo_redo.current->row;
		beforeRedoVal = undo_redo.current->oldValue;
		afterRedoVal = undo_redo.current->value;
		redoCurrent(row, col, beforeRedoVal, afterRedoVal, changesData);/*redo to the current node (head)*/
	}
	else if (undo_redo.current->generateCells == 1 && undoBit == 1) {/*we did generate and then undo*/
		col = undo_redo.current->col;
		row = undo_redo.current->row;
		beforeRedoVal = undo_redo.current->oldValue;
		afterRedoVal = undo_redo.current->value;
		redoCurrent(row, col, beforeRedoVal, afterRedoVal, changesData);/*redo for the first node (head)*/
		while (undo_redo.current->next != NULL && undo_redo.current->next->generateCells == 1) {
			/*while the current node related to the generate command - redoCurrent*/
			col = undo_redo.current->next->col;
			row = undo_redo.current->next->row;
			beforeRedoVal = undo_redo.current->next->oldValue;
			afterRedoVal = undo_redo.current->next->value;
			redoCurrent(row, col, beforeRedoVal, afterRedoVal, changesData);
		}
	}
	else if (undo_redo.current->next->autoCells == 0) {/*if not autofill*/
		col = undo_redo.current->next->col;
		row = undo_redo.current->next->row;
		beforeRedoVal = undo_redo.current->next->oldValue;
		afterRedoVal = undo_redo.current->next->value;
		redoCurrent(row, col, beforeRedoVal, afterRedoVal, changesData);/*redo to the next node*/
	}
	else {
		if (undoBit == 1 && undo_redo.current->autoCells == 1) {/*we did autofill and then undo*/
			col = undo_redo.current->col;
			row = undo_redo.current->row;
			beforeRedoVal = undo_redo.current->oldValue;
			afterRedoVal = undo_redo.current->value;
			redoCurrent(row, col, beforeRedoVal, afterRedoVal, changesData);/*redo to the head*/
			undoBit = 0;
		}
		while ((undo_redo.current->next != NULL) && (undo_redo.current->next->autoCells - 1 == undo_redo.current->autoCells)) {
			/*redo to all the cells related to the autofill command*/
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

/*
This function frees all the sudoku boards and exits the game.
*/
void exitGame() {
	if (currentSudoku != NULL) {
		freeSudoku(currentSudoku);
	}
	if (undo_redo.head != NULL) { 
		freeList(undo_redo.head);
	}
	printf("Exiting...\n");
	exit(0);
}

/*
This function resets the game by execute undo for undo_redo.len times,
and then frees the undo_redo list and initial a new one.
after this function the board and the undo_redo list are empty.
*/
void reset() {
	int i = 0;
	resetPrint = 1;/*we don't want to print the board in undo function*/
	for (i = 0; i < undo_redo.len; i++) {
		if (undoBit == 1) {/*we did undo to the first node-done*/
			break;
		}
		undo();
	}
	resetPrint = 0;
	undoBit = 0;
	freeList(undo_redo.head);
	initList(&undo_redo);
	printf("Board reset\n");
}

/*
This function saves the current coard to a file.
it gets a sudoku to be saved and a path to save it to.
if the mode is edit we check if the board is erroneous or unvalid, if so - we return.
else, we open a file in path, and write the board in it.
*/
void save(Cell** sudoku, char* path) {
	int k, j;
	char* route = (char*)malloc(sizeof(char) * 2048);
	FILE* fd = NULL;
	strcpy(route, path);
	if (mode == 2) {
		if (isBoardErroneous(sudoku) == 1) {
			printf("Error: board contains erroneous values\n");
			free(route);
			return;
		}
		saveGlob = 1;
		if (validate(currentSudoku) == 2) {
			printf("Error: board validation failed\n");
			saveGlob = 0;
			free(route);
			return;
		}
		saveGlob = 0;
	}
	fd = fopen(route, "w");
	if (fd == NULL) {
		printf("Error: File cannot be created or modified\n");
		free(route);
		return;
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
	free(route);
	fclose(fd);
	printf("Saved to: %s\n", path);
}

/*
This function gets a sudoku and executes the autofill command.
it does over the board and check for every cell what are the valid values for it.
if there is one valid value for a cell, it inserts the row, col and the only valid value to the next empty places in the array 'arr'.
afterwards it goes over the array 'arr' and insert the values to the right cells in sudoku bu using 'set' function.
then it updates the autoCells to be the number of cells that had been inserted before it.
*/
void autoFill(Cell** sudoku) {
	int k, j, z, g;
	if (isBoardErroneous(sudoku) == 1) {
		printf("Error: board contains erroneous values\n");
		return;
	}
	int* arr = (int*)malloc((N*N)*sizeof(int)*3);
	int numCounter = 0; /*for counting the number of the possible fills*/
	int counter = 0; /*for counting the number of cell to autofill*/
	int startRow, startCol, tempRow, tempCol, tempVal;
	for (k = 0; k < N; k++) {
		for (j = 0; j < N; j++) {
			if (currentSudoku[k*N + j]->empty == 0) {/*check only the empty cells*/
				startRow = k - k % blockHeight;
				startCol = j - j % blockWidth;
				for (z = 1; z <= N; z++) {
				/*for every empty cell in the sudoku we check which	number between 1-N is valid for him.
				if there is more of	1 option, numCounter wil be > 1*/
					if (isRowValidGame(sudoku, k, j, z) && isColValidGame(sudoku, k, j, z) && isBlockValidGame(sudoku,startRow,startCol, k, j, z)) {
						tempRow = k; tempCol = j; tempVal = z;
						numCounter++;
					}
				}
				if (numCounter == 1) {/*there is only 1 option between 1-N we search for the option and add to the list*/
					arr[counter] = tempRow; arr[counter + 1] = tempCol; arr[counter + 2] = tempVal;
					counter = counter + 3;
				}
			}
			numCounter = 0;
		}
	}
	for (g = 0; g < counter; g=g+3) {
		/*we go over all the cells we found, and insert them to the sudoku using set function*/
		autoFillBit = 1;/*we don't want to print the board in set function*/
		printf("Cell <%d,%d> set to %d\n", arr[g + 1] + 1, arr[g] + 1, arr[g + 2]);
		set(sudoku, arr[g], arr[g + 1], arr[g + 2], NULL);
		undo_redo.tail->autoCells = (g/3)+1;/*updates the node's field for undo and redo commands*/
	}
	if (solvedInSet == 0) {/*we don't want to print when the puzzle is successfully saved*/
		printSudoku(sudoku);
	}
	solvedInSet = 0;
	autoFillBit = 0;
	free(arr);
}

/*
This function gets a sudoku and prints how many solutions it has.
it copies currentSudoku to temp,
then checks the number of solutions by using 'exBackTrack' function.
*/
void num_solutions(Cell** sudoku) {
	int i, j, num;
	if (isBoardErroneous(sudoku) == 1) {
		printf("Error: board contains erroneous values\n");
		return;
	}
	Cell** temp = (Cell**)malloc(sizeof(Cell*)*(N*N));
	for (i = 0; i < N; i++) { /*copy the sudoku that every non empty cell is fixed*/
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
	else if (num > 0) {
		printf("The puzzle has more than 1 solution, try to edit it further\n");
	}
	freeSudoku(temp);
	return;
}

/*
This function gets a sudoku and fix it's cells' erroneousNeib lists.
it goes over the cells in sudoku and check if the value != 0,
if so- it fixes the erroneousNeib lists of the board by using 'erroneousFixAdd' function.
*/
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

/*
This function gets a path to a saved sudoku and loads it.
it initializes a new sudoku of the size that written in the file,
then it goes over the file and inserts the values to the right cells.
change the mode to 1 (solve), and fix the erroneous cells in the board by using 'checkErroneous' function.
*/
void solve(char* path) {
	FILE* fd;
	char buff[256] = "\0";
	char number[256] = "\0";
	int i = 0, dot = 0, k = 0, j;
	int value, col, row;
	Cell** loadBoard;
	mode = 1;
	char* route = (char*)malloc(sizeof(char)*2048);
	strcpy(route, path);
	fd = fopen(route, "r");
	if (fd == NULL) {
		printf("Error: File doesn't exist or cannot be opened\n");
		free(route);
		return;
	}
	if ((fscanf(fd, "%d %d", &col, &row)) != 2) {
		printf("Error: File doesn't exist or cannot be opened\n");
		free(route);
		return;
	}
	if (currentSudoku != NULL) {/* free the memory allocations from the prev game*/
		freeList(undo_redo.head);
		freeSudoku(currentSudoku);
	}
	blockWidth = row; /*n*/
	blockHeight = col; /*m*/
	N = blockHeight * blockWidth;
	loadBoard = (Cell**)malloc(sizeof(Cell*)*(col*col*row*row));
	for (k = 0; k < (row*col); k++) {/*translates the file to a board*/
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
	free(route);
	initList(&undo_redo);
	currentSudoku = loadBoard;
	checkErroneous(currentSudoku);
	printSudoku(currentSudoku);
	fclose(fd);
}

/*
This function initializes an empty board of size 9x9 by default.
it goes over all the cells in the board and inserts 0 as values.
*/
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

/*
This function gets a path to a saved sudoku and loads it.
it initializes a new sudoku of the size that is written in the file,
then it goes over the file and inserts the values to the right cells.
change the mode to 2 (edit), updates 'markError' to be 1 by defalut,
and fix the erroneous cells in the board by using 'checkErroneous' function.
*/
void edit(char* path) {
	FILE* fd;
	char buff[256] = "\0";
	char number[256] = "\0";
	int i = 0, dot = 0, k = 0,j, value, col, row;
	Cell** loadBoard;
	mode = 2;
	char* route = (char*)malloc(sizeof(char) * 2048);
	strcpy(route, path);
	if (*path == '\0') {
		if (currentSudoku != NULL) { /* free the memory allocations from the prev game*/
			freeList(undo_redo.head);
			freeSudoku(currentSudoku);
		}
		loadBoard = generateSudokuGame();
		blockWidth = 3; /*by default*/
		blockHeight = 3;
		N = blockHeight * blockWidth;
	}
	else {
		if ((fd = fopen(route, "r")) == NULL) {
			printf("Error: File cannot be opened\n");
			free(route);
			return;
		}
		if ((fscanf(fd, "%d %d", &col, &row)) != 2) {
			printf("Error: File doesn't exist or cannot be opened\n");
			free(route);
			return;
		}
		if (currentSudoku != NULL) { /* free the memory allocations from the prev game*/
			freeList(undo_redo.head);
			freeSudoku(currentSudoku);
		}
		blockWidth = row;
		blockHeight = col;
		N = blockHeight * blockWidth;
		loadBoard = (Cell**)malloc(sizeof(Cell*)*(col*col*row*row));
		for (k = 0; k < (row*col); k++) {/*translates a file to a board*/
			for (j = 0; j < (col*row); j++) {
				fscanf(fd, "%s", buff);
				while (buff[i] != '\0') {
					if (buff[i] != '.') {
						number[i] = buff[i];
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
				dot = 0;
			}
		}
		fclose(fd);
	}
	free(route);
	markError = 1;
	initList(&undo_redo);
	currentSudoku = loadBoard;
	checkErroneous(currentSudoku);
	printSudoku(currentSudoku);
}

/*
This function gets a sudoku and clear the board.
it goes over all the cells, changes the value to 0 and updates the cells to be empty.
*/
void clearBoard(Cell** sudoku) {
	int i = 0, j = 0;
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			sudoku[i*N + j]->empty = 0;
			sudoku[i*N + j]->value = 0;
		}
	}
}

/*
This function gets x and y and genertes a board with y values != 0.
it has 1000 chances to randomly choose x different cells and for each one randomly choose a valid value,
by using 'isRowValidGame', 'isColValidGame' and 'isBlockValidGame' functions,
and to solve the current sudoku by using 'ILPSolver' function.
if there is a solution, it randomly chooses y different cells and insert them to currentSudoku.
*/
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
			for (j = 1; j <= N; j++) {/*countinf the number of possible values for current cell*/
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
				generateGlobal = 1;/*we don't want to insert a node into undo_redo list in set function*/
				generateSudokuPrint = 1;/*we don't want to print the vboard in set function*/
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
		freeSudoku(solvedSudoku);
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
			generateSudokuPrint = 1;/*we don't want to print the board in set function*/
			set(currentSudoku, row, col, solvedSudoku[row*N + col]->value, NULL);
			generateSudokuPrint = 0;
			undo_redo.tail->generateCells = 1;
		}
	}
	if (ret == 1) {
		freeSudoku(solvedSudoku);
	}
	printSudoku(currentSudoku);
	return;
}

/*
This function checks which command we need to execute,
and calls it's function accordingly.
*/
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

/*
This function frees all the memory resources that we used to our sudoku board.
this function frees every cell in the board and after that frees the matrix sudoku.
*/
void freeSudoku(Cell** sudoku) {
	int i, j;
	if (sudoku != NULL) {
		for (i = 0; i < N; i++) {
			for (j = 0; j < N; j++) {
				if (sudoku[i*N + j]->fixed == 0) {
					if (sudoku[i*N + j]->erroneousNeib.head) {
						freeList(sudoku[i*N + j]->erroneousNeib.head);
					}
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

