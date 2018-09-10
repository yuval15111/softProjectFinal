#ifndef ILPAlgo_H_
#define ILPAlgo_H_


/*This function frees all memory allocations except of 'sol'

void freeAll(double* lb, double* val, char* verType, int* index);


This function writes the solution in sol to the sudoku:
it checks for each k s.t 1<=k<=N wether sol in k index equals 1,
if so -> insert k as the value of the same index in the sudoku

void writeSolToBoard(double* sol);


int exitILP(GRBenv** env, GRBmodel** model, int flag, int optimStat, double* sol, int ThreeDMatrixSize);

void addVaribles(double *lb, char *verType);

int oneValPerBlockCon(GRBmodel model, int index, double val);

int oneValPerRowCon(GRBmodel model, int index, double val);

int oneValPerColCon(GRBmodel model, int index, double val);


*/

/*This function uses the ILP algorithm and tries to solve currentSudoku.
@return
-1 <- if an error occured in the program
1 <- if the board is solvable, it solves the sudoku and saves it in solvedSudoku
2 <- if the board is unsolvable. it doesn't change solvedSudoku.
*/
int ILPSolver();

#endif /*ILPAlgo_H_*/
