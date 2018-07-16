
/*
The module game contain the board that we are playing on and the solution of this
borad. The module is used to manipulate the game status, validate, set, restart,
exit game, printing sudoku board ect.
The game takes place in this module.
*/

#ifndef GAME_H_
#define GAME_H_
#include<stdbool.h>
int mode; /*1 - solve command and 2 - edit and 0 - init*/
int idCommand; /*1 - solve command and 2 - edit and 0 - else 3-save*/
int blockWidth, blockHeight, N;
int numOfEmptyCells;

typedef struct node {
	int row, col, value, oldValue;
	struct node* next;
	struct node* prev;
}node;

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
This function get the current game Sudoku and checks if there is any solution of this board
by using determenistic Backtrack algorithm
If there is no solution it prints a message to the user,
otherwise, it replace the solved sudoku that save the solution of the current sudoku board
and print a Appropriate message to the user.
*/
void validate(Cell** currentSudoku);

/*
This function get row and column.
@return
the value of the cell in solvedSudoku[row][column] that saved the solution of the current board.
*/
void hint(int row, int col);

/*
This function checks if the board is full:
@return
True - if the board is full (and correctly)
Flase - the board isnt full
*/
bool isGameOver(Cell** sudoku);

/*
This function gets value, row and column and checks if it is valid to put the value in currentSudoku[row][column]
if it is - cell->value = val and checks if the game is over.
If the game is over we are now enable only command of exit or restart
otherwise - we contine as usual.
If the val isn't valid we print to the user an appropriate message.
*/
void set(Cell** currentSudoku, int row, int col, int val, char* oldCommand);

/*
This function print sudoku boards in a beatiful way
*/
void printSudoku(Cell** sudoku);

/*
This function update the address of solvedSudoku that saved the solution of current sudoku in game.c module
boardGeneration is the address that we got from puzzleGeneration function (solver module).
*/
void getSolvedSudoku(Cell** boardGeneration);

/*
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

/*
This function free all the sudoku boards and call again to puzzleGeneration with the
function initNumberOfHints() (in order to get the number of hints that the user wants for his
new game.
then, calls playgame func.
*/
void restart();

/*
The goal of the function is to receive commands and continue to play according to this commands
from the user.
This function is actually the place that the game occure.
*/
void playGame();



#endif /*GAME_H_*/

