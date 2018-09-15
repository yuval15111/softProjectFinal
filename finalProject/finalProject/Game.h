
/*
The module game contain the board that we are playing on and the solution of this
borad. The module is used to manipulate the game status, validate, set, restart,
exit game, printing sudoku board ect.
The game takes place in this module.
*/

#ifndef GAME_H_
#define GAME_H_
#include<stdbool.h>
/*mode: 1 - solve command and 2 - edit and 0 - init*//*######delete??#######*/
int idCommand; /*1 - solve command and 2 - edit and 0 - else 3-save*/
int blockHeight, blockWidth, N; /*blockHeight=m; blockWidth=n; N=n*m*/
int numOfEmptyCells; /*the number of empty cells in the sudoku*/

/*
The struct node will respresent every 'cell' in the  doubly linked list below
it's field:
row- the row of the node's cell
col- the coulmn of the node's cell
value- the value of the node's cell
oldVal- the previous value of the node's cell
next- the next node in the linked list
prev- the previous node in the linked list
*/
typedef struct node {
	int row, col, value, oldValue, autoCells, generateCells;
	struct node* next;
	struct node* prev;
}node;

/*
The struct linkedeList will represent a doubly linked list for the undo/redo list.
We will also use this struct as a field for each cell in the sudoku.
it's fields:
head- a pointer to the first node in the linked list
current- a pointer to the current node (for undo/redo list)
tail- a pointer to the last node in the linked list
len- the length of the linked list
*/
typedef struct linkedList {
	node* head;
	node* current;
	node* tail;
	int len;
}linkedList;

/*
The struct cell will represent every cell in the sudoku.
his fields:
value - the value of the cell
fixed - 0 -> the cell isnt fixed, 1 - otherwise
empty - 0 -> the cell isnt empty, 1 - otherwise
arr - this field is being used in the random backtrack algorithm:
arr[0] - the number of candidate that can be in the cell value
arr[1] - arr[(blockHeight*blockWidth) + 1]: 1 - if the index (that represent the value)
is a candidate and 0 - otherwise
*/
typedef struct cell_t {
	int value;
	int fixed; /*0 meaning not used*/
	int empty;
	int arr[10];
	int erroneous;
	linkedList erroneousNeib;
}Cell;

/*
This function get value and create the cell with the value and the other fields will be default :
cell->value = value;
cell->fixed = 0;
cell->empty = 0;
cell->arr[0] = 0;
*/
Cell* createCell(int value);

/*
This function checks if the num parameter is valid in the row of the sudoku.
(checks if num is already exist in the row of the sudoku)
@return
True - the num parameter doesnt exist
False - otherwise
*/
bool isRowValidGame(Cell** sudoku, int row, int col, int num);

/*
This function checks if the num parameter is valid in the column of the sudoku.
(checks if num is already exist in the column of the sudoku)
@return
True - the num parameter doesnt exist
False - otherwise
*/
bool isColValidGame(Cell** sudoku, int row, int col, int num);

/*
This function checks if the num parameter is valid in the block of the sudoku.
(checks if num is already exist in the block of the sudoku)
@return
True - the num parameter doesnt exist
False - otherwise
*/
bool isBlockValidGame(Cell** sudoku, int startRow, int startCol, int row, int col, int num);

/*
The goal of the function is to receive commands and continue to play according to this commands
from the user.
This function is actually the place that the game occure.
*/
void playGame();


#endif /*GAME_H_*/

