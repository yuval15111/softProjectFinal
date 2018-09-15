#ifndef ILPAlgo_H_
#define ILPAlgo_H_


/*This function uses the ILP algorithm and tries to solve currentSudoku.
@return
-1 <- if an error occured in the program
1 <- if the board is solvable, it solves the sudoku and saves it in solvedSudoku
2 <- if the board is unsolvable. it doesn't change solvedSudoku.
*/
int ILPSolver();

#endif /*ILPAlgo_H_*/
