/*
The module solver implements the Backtrack algorithms.
In addition, this module generates the sudoku boards without any connection to the user.
*/

#ifndef SOLVER_H_
#define SOLVER_H_
#include<stdbool.h>
#include "Game.h"

/*
the range of our sudoku values.
*/
#define rangeOfNum blockHeight*blockWidth

/*
creates a new sudoku by initialize a two dimension array of Cells
*/
Cell** generateSudoku();

/*
the function gets the current sudoku and the potential parameter to insert.
it checks if there is no other cell in the current row with cell.value=num
@return
True- the num parameter doesn't exist
False- otherwise
*/
bool isRowValid(Cell** sudoku, int num);

/*
the function gets the current sudoku and the potential parameter to insert.
it checks if there is no other cell in the current column with cell.value=num
@return
True- the num parameter doesn't exist
False- otherwise
*/
bool isColValid(Cell** sudoku, int num);

/*
the function gets the current sudoku, the indexes of first cell in the block,
and the potential parameter to insert.
it checks if there is no other cell in the current block with cell.value=num
@return
True- the num parameter doesn't exist
False- otherwise
*/
bool isBlockValid(Cell** sudoku, int startRow, int startCol, int num);

/*
the function gets the current sudoku and the potential parameter to insert.
it checks if num is a valid parameter for the current cell.value by using the functhions:
isValidRow, isValidCol and isValidBlock.
@return
True- if isValidRow, isValidCol and isValidBlock returns true
False- otherwise
*/
bool isValidNum(Cell** sudoku, int num);

/*
the function gets the current sudoku and checks if there is an empty cell in the sudoku.
in addition it updates the global variables row&col to be the indexes of the first empty cell that is found.
@return
True- if there is an empty cell
False- otherwise
*/
bool findNextEmptyCell(Cell** sudoku);

/*
the function changes the global col and row to the indexes of the previous cell in the sudoku
*/
void stepBack();

/*
the function is the recursive determenistic backtrack algorithm that we learned.
it gets the current sudoku.
the algorithm works from left to right and top down:
it checks if there is an empty cell and updates the global col&row to the indexes of the next empty cell by using findNextEmptyCell func.
for this empty cell we check what is the first valid parameter the we haven't used yet from 1 to 9,
by using isValidNum func, and insert it to the cell.
if there is no valid parameter to a cell, we go backwords by using the func stepBack, and continue to the next valid parameter.
this function changes the current sudoku to it's solution (if possible) in place.
@return
True- there is solution for this sudoku
False- otherwise
*/
bool detBacktrackRec(Cell** sudoku);

/*
this is the shell function of the detBacktrackRec function.
it gets the current sudoku and copy it. from now on, it will work on the copy.
the function checks what the recursive algorithm returns:
@return
copy sudoku - if it returned true
NULL - if it returned false
*/
Cell** determenisticBacktrack(Cell** sudoku);

/*
the function updates the array field (which indicates the valid values we can insert to the cell).
it gets the current sudoku and uses the global row&col to get to the current cell.
for every number i from 1 to 9 we check if this number is valid for this cell by using isValidNum function.
if the number is valid - cell.array[i]=1
otherwise- cell.array[i]=0.
*/
void updateArrayForCell(Cell** boardGeneration);

/*
the function gets a cell.array.
it chooses a number randomly from 1 to array[0] (which is the number of valid values for the current cell).
@return
the random number
*/
int chooseRandomNumberToRemove(int* arrayOfNumbers);

/*
the function gets the current sudoku and the random number we got from chooseRandomNumberToRemove function.
it uses the global col&row and go over the current cell.array.
we stop after we see for rand times the value 1,
change the value of this cell from 1 to 0 (meaning we removed this option from the valid values for cell)
and update the value of the current cell to this index.
*/
void removeNumberFromArray(Cell** board, int rand);

/*
the function gets a cell.array.
it is called only when there is one valid value for this cell -
in this case we find the only index in the array with the value 1.
@return
i s.t cell.array[i]==1
*/
int findIndexToRemove(int* arr);

/*
the function the random backtrack algorithm.
it gets the current sudoku, which is an empty board.
the algorithm works from left to right and top down:
the algorithm looks for the next empty cell by using findNextEmptyCell function,
and updates it's array field by using the function updateArrayForCell.
than it choose a number fron 1 to 9 randomly by using the function chooseRandomNumberToRemove (in case there is only one value possible it chooses it),
insert this num as the cell.value and remove it from it's array.
this function changes the empty board in place to a solved one.
@return
True- the sudoku built
False- otherwise
*/
bool randomBacktrack(Cell** boardGeneration);

/*
this is the shell function for generelize a game board.
it gets the number of hints we got from the user.
it creates 2 empty boards,
uses randomBacktrack function to solve one of them and send it to 'game' module to save the solution by using getSolvedSudoku function.
then it's using the getHintsBoard function to create the game board that will be sent to the user (the other board).
aftewards it saves the surrent sudoku in the 'game' module by using the function getSudokuWithHints,
and finally print the sudoku by using printSudoku function.
*/
void puzzleGeneration(int hints);

/*
the function gets the number of hints the user chose, the solved sudoku and an empty board.
for jints times, the function randomly chooses a row and col (->a cell),
checks if sudokuWithHints[row][col].fixed==0 and if so it inserts the solvedSudoku[row][col] values into this cell.
this function changes the sudokuWithHints in place, and in the end it will be the game board.
*/
void getHintsBoard(int hints, Cell** solvedSudoku, Cell** sudokoWithHints);

#endif /*SOLVER_H_*/

