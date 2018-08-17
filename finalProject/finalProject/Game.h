
/*
The module game contain the board that we are playing on and the solution of this
borad. The module is used to manipulate the game status, validate, set, restart,
exit game, printing sudoku board ect.
The game takes place in this module.
*/

#ifndef GAME_H_
#define GAME_H_
#include<stdbool.h>
/*mode: 1 - solve command and 2 - edit and 0 - init*/
int idCommand; /*1 - solve command and 2 - edit and 0 - else 3-save*/
int blockWidth, blockHeight, N; /*blockWidth=m; blockHeight=n; N=n*m*/
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
	int row, col, value, oldValue, autoCells;
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
arr[1] - arr[(blockWidth*blockHeight) + 1]: 1 - if the index (that represent the value)
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
This function gets a node in the doubly linked list and free all the memory allocations starts by this node
(free 'headNode' and all the nodes after it in the doubly linked list)
@return
the number of cells the function did free to
*/
int freeList(node* headNode);

/*
This function initiallizes the doubly linked list: 
allocates space in memory for the 'head' field
init head.prev and head.next as NULL
init the len field to 0
define tail and current to be the head
*/
void initList(linkedList* l);

/*
This function adds a node to the end of a linked list,
it gets a doubly linked list and the fields of the new node:
allocates memory space for the node if needed,
updates the tail and current to the new node,
update the len field
*/
void addNode(linkedList* list, int row, int col, int val, int oldVal);

/*
This function deletes a specific node from the erroneousNeib doubly linked list
it gets the cell of the node to be deleted, a doubly linked list and the node to be deleted:
if origNode is the only node in the linked list, it frees the node and creates a new erroneousNeib list
else it updates the list and its len and frees origNode
*/
void deleteErrorNeibNode(Cell* cell, linkedList* list, node* origNode);

/*
This function deletes all the nodes from 'nodeToBeDeleted' to the tail in the doubly linked list:
if nodeToBeDeleted is the head it frees the entire list by using 'freeList' function and updates the len to 0
else, it updates the last node to be NULL and deletes the node from nodeToBeDeleted, and updates the len
*/
void deleteListFrom(node* nodeToBeDeleted);

/*
This function get value and create the cell with the value and the other fields will be default :
cell->value = value;
cell->fixed = 0;
cell->empty = 0;
cell->arr[0] = 0;
*/
Cell* createCell(int value);

/*
This function get a cell and copy his fields
@return
copy of the cell
*/
Cell* copyCell(Cell* cell);

/*
This function prints the separators between the blocks of the sudoku
*/
void printSeparator();

/*
This function prints the sudoku boards as instructed using printSeparator func
*/
void printSudoku(Cell** sudoku);

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

/*##############change after doing ILP##############################
This function get the current game Sudoku and checks if there is any solution of this board
by using determenistic Backtrack algorithm
If there is no solution it prints a message to the user,
otherwise, it replace the solved sudoku that save the solution of the current sudoku board
and print a Appropriate message to the user.
*/
int validate(Cell** currentSudoku);

/*######################change after doing ILP#####################
This function get row and column.
@return
the value of the cell in solvedSudoku[row][column] that saved the solution of the current board.
*/
void hint(int row, int col);

/*#####################delete?############################
This function checks if the board is full:
@return
True - if the board is full (and correctly)
Flase - the board isnt full
*/
bool isGameOver(Cell** sudoku);

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
void checkErrorRow(int row, int col, int val);

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
void checkErrorCol(int row, int col, int val);

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
void checkErrorBlock(int row, int col, int val);

/*
This function handles all the erroneous cells after inserting a new value to the sudoku:
it gets the row, col and value of the new cell in the sudoku
it uses 'checkErrorRow', 'checkErrorCol' and 'checkErrorBlock' functions.
*/
void erroneousFixAdd(int row, int col, int val);

/*
This function handles all the erroneous cells after deleting value from the sudoku:
it gets the row, col and value of the deleted cell
the function goes over the erroneousNieb list of the deleted cell, and for every cell C in that list:
1. delete C's node from the deleted cell's list
2. if the C is not fixed: 
   2.1 delete the deleted cell's node from the C's list
   2.2 update erroneousNieb len of C and if needed update it's erroneous field (if len=0)
*/
void erroneousFixDel(int row, int col, int val);

/*
This function checks if there is an erroneous cell in the sudoku:
it gets a sudoku, 
it goes over all the cells in the sudoku and checks it's erroneous field
@return
1 if erroneous, 0 if not erroneous
*/
int isBoardErrorneus(Cell** sudoku);

/*
This function counts the number of empty cells in the sudoku:
it gets a sudoku,
it goes over all the cells in the sudoku and checks it's empty field,
if the cell is empty - adds the counter 1
@return
the counter
*/
int checkNumOfEmptyCells(Cell** sudoku);

/*
This function gets value, row and column and checks if it is valid to put the value in currentSudoku[row][column]
if it is - cell->value = val and checks if the game is over.
If the game is over we are now enable only command of exit or restart
otherwise - we contine as usual.
If the val isn't valid we print to the user an appropriate message.
*/
void set(Cell** sudoku, int row, int col, int val, char* oldCommand);

void undo();

/*
This function update the address of solvedSudoku that saved the solution of current sudoku in game.c module
boardGeneration is the address that we got from puzzleGeneration function (solver module).
*/
void getSolvedSudoku(Cell** boardGeneration);

/*####################################### delete delete delete ####################################
This function update the address of currentSudoku that saved the board with hints in game.c module
sudokuWithHints is the address that we got from puzzleGeneration function (solver module).
*/
void getSudokuWithHints(Cell** sudokuWithHints);

/*
This function checks which command we need to execute and calls his function accordingly.

*/
void doCommand(char* command);
/*
This function free all the memory resources that we used to our sudoku board.
this function free every cell in the board and after that free the matrix sudoku.
*/
void freeSudoku(Cell** sudoku);

/*
This function free all the sudoku boards, say goodbye to the user and exit
*/
void exitGame();

/*########################## maybe change to reset - otherwise delete #####################
This function free all the sudoku boards and call again to puzzleGeneration with the
function initNumberOfHints() (in order to get the number of hints that the user wants for his
new game.
then, calls playgame func.
*/
void reset();

void solve(char* path);

void edit(char* path);


/*
The goal of the function is to receive commands and continue to play according to this commands
from the user.
This function is actually the place that the game occure.
*/
void playGame();

void num_solutions(Cell ** sudoku);


#endif /*GAME_H_*/

