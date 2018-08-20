
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

/*
This function gets the current game Sudoku board and checks if there is any solution of this board
by using ILP in module 'ILPAlgo'
@return
0 -> the board contains erroneous values
1 -> the board is solvable (we found a solution using ILP)
2 -> the board is unsolvable or the ILP algorithm didn't work
*/
int validate(Cell** currentSudoku);

/*
This function gets a row and a column, solve the current coard by using ILP,
and print the value of the cell that the ILP found.
@return
nothing -> when the currentsudoku is erroneous or the cell is fixed or the cell is not empty 
			or the board is unsolvable
solvedSudoku[row][column] that saved the solution of the current board -> if the board is solvable
*/
void hint(int row, int col);

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
void set(Cell** sudoku, int row, int col, int val, char* oldCommand);

/*
This function makes undo to the current pointer of the undo_redo list:
it checks what is the current cell parameters (col, row, before undo value, after undo value),
changes the value of that cell to the after undo value,
and prints the the undo move.
it changes the current pointer to be the prev node.
^^if the prev node is NULL (we made undo to the first node) -> undoBit=1.
it fixes the cells' erroneousNeib lists by using 'erroneousFixDel' and 'erroneousFixAdd' functions.
*/
void undoCurrent();

/*
This function executes the 'undo' command:
if undo_redo len==0 -> there are no moves to undo.
if the autoCells of the current pointer of the undo_redo list > 0 -> it means that we do undo to autofill:
	we move the current pointer to be the prev node for autocCells-1 times,
	then we do undo to the current pointer by using 'udoCurrent' function, and change the current pointer to the next node,
	then we change the current pointer again to it's prev for autoCells times.
else -> we have only one node to undo, and we execute it by 'undoCurrent' function
*/
void undo();

/*
This function executes redo for the current pointer in undo_redo list.
it gets a row, col, before redo value and after redo value.
it prints the redo move as asked,
it changes the current pointer to be the next node:
^^if undoBit==1 and the next node is NULL -> we don't change the current pointer.
else if we have a next node -> we change it.
afterwards we fix the cells' erroneousNeib lists by using 'erroneousFixDel' and 'erroneousFixAdd' functions.
*/
void redoCurrent(int row, int col, int beforeRedoVal, int afterRedoVal);

/*
###################not done yet#######################3
This function do the redo command:
it checks if there are no moves to redo.
if the undoBit==1 and the next node is NULL -> we did undo to the first node in undo_redo list,
so we don't change the current pointer and updates the cell's new parameters.
else if the next node's autoCells == 1 -> we re
*/
void redo();

/*
This function frees all the sudoku boards and exits the game.
*/
void exitGame();

/*
This function resets the game by execute undo for undo_redo.len times, 
and then frees the undo_redo list and initial a new one.
after this function the board and the undo_redo list are empty. 
*/
void reset();

/*
This function saves the current coard to a file.
it gets a sudoku to be saved and a path to save it to.
if the mode is edit we check if the board is erroneous or unvalid, if so - we return.
else, we open a file in path, and write the board in it.
*/
void save(Cell** sudoku, char* path);

/*
This function gets a sudoku and executes the autofill command.
it does over the board and check for every cell what are the valid values for it.
if there is one valid value for a cell, it inserts the row, col and the only valid value to the next empty places in the array 'arr'.
afterwards it goes over the array 'arr' and insert the values to the right cells in sudoku bu using 'set' function.
then it updates the autoCells to be the number of cells that had been inserted before it.

*/
void autoFill(Cell** sudoku);

/*
This function gets a sudoku and prints how many solutions it has.
it copies currentSudoku to temp,
then checks the number of solutions by using 'exBackTrack' function.
*/
void num_solutions(Cell ** sudoku);

/*
This function gets a sudoku and fix it's cells' erroneousNeib lists.
it goes over the cells in sudoku and check if the value != 0,
if so- it fixes the erroneousNeib lists of the board by using 'erroneousFixAdd' function.
*/
void checkErroneous(Cell** sudoku);

/*
This function gets a path to a saved sudoku and loads it.
it initializes a new sudoku of the size that written in the file,
then it goes over the file and inserts the values to the right cells.
change the mode to 1 (solve), and fix the erroneous cells in the board by using 'checkErroneous' function.
*/
void solve(char* path);

/*
This function initializes an empty board of size 9x9 by default.
it goes over all the cells in the board and inserts 0 as values.
*/
Cell** generateSudokuGame();

/*
This function gets a path to a saved sudoku and loads it.
it initializes a new sudoku of the size that is written in the file,
then it goes over the file and inserts the values to the right cells.
change the mode to 2 (edit), updates 'markError' to be 1 by defalut,
and fix the erroneous cells in the board by using 'checkErroneous' function.
*/
void edit(char* path);

/*
This function gets a sudoku and clear the board.
it goes over all the cells, changes the value to 0 and updates the cells to be empty. 
*/
void clearBoard(Cell** sudoku);

/*
This function gets x and y and genertes a board with y values != 0.
it has 1000 chances to randomly choose x different cells and for each one randomly choose a valid value, 
by using 'isRowValidGame', 'isColValidGame' and 'isBlockValidGame' functions,
and to solve the current sudoku by using 'ILPSolver' function.
if there is a solution, it randomly chooses y different cells and insert them to currentSudoku.
*/
void generate(int x, int y);

/*
This function checks which command we need to execute,
and calls it's function accordingly.
*/
void doCommand(char* command);

/*
This function frees all the memory resources that we used to our sudoku board.
this function frees every cell in the board and after that frees the matrix sudoku.
*/
void freeSudoku(Cell** sudoku);


/*
The goal of the function is to receive commands and continue to play according to this commands
from the user.
This function is actually the place that the game occure.
*/
void playGame();




#endif /*GAME_H_*/

