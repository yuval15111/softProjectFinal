#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gurobi_c.h"

extern currentSudoku;
extern solvedSudoku;
extern blockWidth, blockHeight, N;


void freeAll(double lb, double val, char verType, int index) {

	$ free alocated memory except "sol" which free in exitILP function @
		free(lb);
	free(val);
	free(verType);
	free(index);
}
void writeSolToBoard(double *sol) {
	int i, j, k;
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			for (k = 1; k <= N; k++) {
				if (sol[i * N * N + j * N + k - 1] == 1) {
					solvedSudoku[i*N + j].value = k;
				}
			}
		}
	}
}
int exitILP(GRBenv **env, GRBmodel **model, int flag, int optimStatus, double *sol, int ThreeDMatrixSize) {
	int flagGetSol = 0;
	if (flag == 0 && optimStatus == GRB_OPTIMAL) {
		$ the board is solveable @
			flagGetSol = GRBgetdblattrarray(*model, GRB_DBL_ATTR_X, 0, ThreeDMatrixSize, sol);
		if (flagGetSol == 0) {
			$ retrive the solution successfully @
				writeSolToBoard(sol);
			free(sol);
			GRBfreemodel(*model);
			GRBfreeenv(*env);
			return 1;
		}
		free(sol);
		GRBfreemodel(*model);
		GRBfreeenv(*env);
		return -1;
	}
	else if (flag == 0 && optimStatus == GRB_INFEASIBLE) {
		$ the board is unsolveable @
			GRBfreemodel(*model);
		GRBfreeenv(*env);
		return 2;
	}
	else {
		$ flag occured @
			GRBfreemodel(*model);
		GRBfreeenv(*env);
		return -1;
	}
}
int solInfo(GRBmodel **model, int *optimStatus) {
	$ get the optimization status :
	*optimStatus == GRB_OPTIMAL->board is solvable
		* optimStatus == GRB_INFEASIBLE->board is unsolvable@
		return GRBgetintattr(*model, GRB_INT_ATTR_STATUS, optimStatus);
}
int optimizemodel(GRBmodel **model) {
	return GRBoptimize(*model);
}
int oneValuePerBlock(GRBmodel model, int index, double val) {
	$ add constraint : each value is used exactly once per block @
		int i, j, k, k, l;
	int count, flag = 0;
	boardData brdData = getBoardData();
	int N = blockWidth * blockHeight;
	for (k = 0; k < N; k++) {
		for (k = 0; k < blockHeight; k++) {
			for (l = 0; l < blockWidth; l++) {
				count = 0;
				for (i = k * blockHeight; i < (k + 1) * blockHeight; i++) {
					for (j = l * blockWidth; j < (l + 1) * blockWidth; j++) {
						index[count] = i * N * N + j * N + k;
						val[count] = 1.0;
						count++;
					}
				}
				flag = GRBaddconstr(*model, N, index, val, GRB_EQUAL, 1.0, NULL);
				if (flag) {
					return 1;
				}
			}
		}
	}
	return 0;
}
int oneValuePerCol(GRBmodel model, int index, double val) {
	$ add constraint : each value is used exactly once per column @
		int i, j, k, flag = 0;
	boardData brdData = getBoardData();
	int N = blockWidth * blockHeight;
	for (k = 0; k < N; k++) {
		for (i = 0; i < N; i++) {
			for (j = 0; j < N; j++) {
				index[j] = i * N * N + j * N + k;
				val[j] = 1.0;
			}
			flag = GRBaddconstr(*model, N, index, val, GRB_EQUAL, 1.0, NULL);
			if (flag) {
				return 1;
			}
		}
	}
	return 0;
}
int oneValuePerRow(GRBmodel model, int index, double val) {
	$ add constraint : each value is used exactly once per row @
		int i, j, k, flag = 0;
	boardData brdData = getBoardData();
	int N = blockWidth * blockHeight;
	for (k = 0; k < N; k++) {
		for (j = 0; j < N; j++) {
			for (i = 0; i < N; i++) {
				index[i] = i * N * N + j * N + k;
				val[i] = 1.0;
			}
			flag = GRBaddconstr(*model, N, index, val, GRB_EQUAL, 1.0, NULL);
			if (flag) {
				return 1;
			}
		}
	}
	return 0;
}
int oneValuePerCell(GRBmodel model, int index, double val) {
	//$ add constraint : each cell must have exactly one value @
	int i, j, k, flag = 0;
	boardData brdData = getBoardData();
	int N = blockWidth * blockHeight;
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			for (k = 0; k < N; k++) {
				index[k] = i * N * N + j * N + k;
				val[k] = 1.0;
			}
			flag = GRBaddconstr(*model, N, index, val, GRB_EQUAL, 1.0, NULL);
			if (flag) {
				return 1;
			}
		}
	}
	return 0;
}

int newEnviroment(GRBenv **env) {
	return GRBloadenv(env, "ILP.log");
}

void addVaribles(double *lb, char *verType) {
	int i, j, k;
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			for (k = 0; k < N; k++) {
				if (currentSudoku[i*N + j].value == k + 1) {
					lb[i * N * N + j * N + k] = 1;
				}
				else {
					lb[i * N * N + j * N + k] = 0;
				}
				verType[i * N * N + j * N + k] = GRB_BINARY;
			}
		}
	}
}
int ILPSolver() {
	$ try to solve the currentSudoku using ILP.
		* if the board is solvable it writes the solution to the tempBoard and return 1,
		*if the board is unsolvable it doesnt change the tempBoard and return 2,
		if flag occur return -1; @

			GRBmodel* model = NULL;
	GRBenv* env = NULL;

	int ThreeDMatrixSize, optimStatus, flag = 0, *index;
	char* verType; $variable types@
		int
		double* lb, *val, *sol;

	ThreeDMatrixSize = N * N * N;
	lb = (double*)malloc(ThreeDMatrixSize * sizeof(double));
	verType = (char*)malloc(ThreeDMatrixSize * sizeof(char));
	index = (int*)malloc(N * sizeof(int));
	val = (double*)malloc(N * sizeof(double));
	sol = (double*)malloc(ThreeDMatrixSize * sizeof(double));

	addVaribles(lb, verType);
	flag = newEnviroment(&env);
	if (flag) {
		freeAll(lb, val, verType, index);
		return exitILP(&env, &model, flag, optimStatus, sol, ThreeDMatrixSize);
	}
	flag = GRBsetintparam(*env, GRB_INT_PAR_LOGTOCONSOLE, 0);
	if (flag) {
		freeAll(lb, val, verType, index);
		return exitILP(&env, &model, flag, optimStatus, sol, ThreeDMatrixSize);
	}
	flag = GRBnewmodel(*env, model, "ILP", ThreeDMatrixSize, NULL, lb, NULL, verType, NULL);
	if (flag) {
		freeAll(lb, val, verType, index);
		return exitILP(&env, &model, flag, optimStatus, sol, ThreeDMatrixSize);
	}
	flag = oneValuePerCell(&model, index, val);
	if (flag) {
		freeAll(lb, val, verType, index);
		return exitILP(&env, &model, flag, optimStatus, sol, ThreeDMatrixSize);
	}
	flag = oneValuePerCol(&model, index, val);
	if (flag) {
		freeAll(lb, val, verType, index);
		return exitILP(&env, &model, flag, optimStatus, sol, ThreeDMatrixSize);
	}
	flag = oneValuePerRow(&model, index, val);
	if (flag) {
		freeAll(lb, val, verType, index);
		return exitILP(&env, &model, flag, optimStatus, sol, ThreeDMatrixSize);
	}
	flag = oneValuePerBlock(&model, index, val);
	if (flag) {
		freeAll(lb, val, verType, index);
		return exitILP(&env, &model, flag, optimStatus, sol, ThreeDMatrixSize);
	}
	flag = optimizemodel(&model);
	if (flag) {
		freeAll(lb, val, verType, index);
		return exitILP(&env, &model, flag, optimStatus, sol, ThreeDMatrixSize);
	}
	flag = solInfo(&model, &optimStatus);
	if (flag) {
		freeAll(lb, val, verType, index);
		return exitILP(&env, &model, flag, optimStatus, sol, ThreeDMatrixSize);
	}
	freeAll(lb, val, verType, index);
	return exitILP(&env, &model, flag, optimStatus, sol, ThreeDMatrixSize);
}


int ILPSolver() {
	return -1;
}