#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Game.h"
#include "gurobi_c.h"

extern Cell** currentSudoku;
extern Cell** solvedSudoku;
extern int blockWidth, blockHeight, N;


void freeAll(double* lb, double* val, char* verType, int* index) {
	printf("freeAll");
	free(lb);
	free(val);
	free(verType);
	free(index);
}

void writeSolToBoard(double* sol) {
	int i, j, k;
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			for (k = 0; k < N; k++) {
				if (sol[i * N * N + j * N + k] == 1) {
					solvedSudoku[i*N + j]->value = k + 1;
				}
			}
		}
	}

}

int exitILP(GRBenv** env, GRBmodel** model, int flag, int optimStat, double* sol, int ThreeDMatrixSize) {
	int solAns = 0;
	printf("exitILP");
	if (flag == 0 && optimStat == GRB_OPTIMAL) { /* the board is solveable */
		solAns = GRBgetdblattrarray(*model, GRB_DBL_ATTR_X, 0, ThreeDMatrixSize, sol);
		if (solAns == 0) { /* retrive the solution successfully */
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
	else if (flag == 0 && optimStat == GRB_INFEASIBLE) { /* the board is unsolveable */
		GRBfreemodel(*model);
		GRBfreeenv(*env);
		return 2;
	}
	else { /* a problem occured */
		GRBfreemodel(*model);
		GRBfreeenv(*env);
		return -1;
	}
}


int oneValPerCellCon(GRBmodel** model, int* index, double* val) {
	/* each cell have exactly one value */
	int i, j, k, flag = 0;
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

int oneValPerColCon(GRBmodel** model, int* index, double* val) {
	/* constraint : each value is used exactly once in each column */
		int i, j, k, flag = 0;
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

int oneValPerRowCon(GRBmodel** model, int* index, double* val) {
	/* constraint : each value is used exactly once in each row */
		int i, j, k, flag = 0;
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

int oneValPerBlockCon(GRBmodel** model, int* index, double* val) {
	/* constraint : each value is used exactly once in each block */
		int i, j, e, k, l;
	int count, flag = 0;
	for (e = 0; e < N; e++) {
		for (k = 0; k < blockHeight; k++) {
			for (l = 0; l < blockWidth; l++) {
				count = 0;
				for (i = k * blockHeight; i < (k + 1) * blockHeight; i++) {
					for (j = l * blockWidth; j < (l + 1) * blockWidth; j++) {
						index[count] = i * N * N + j * N + e;
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

void addVaribles(double *lb, char *verType) {
	/* if a cell value equals k->insert 1 at this index in lb matrix
	else->insert 0 at this index in lb matrix */
		int i, j, k;
	for (i = 0; i < N; i++) {
		for (j = 0; j < N; j++) {
			for (k = 0; k < N; k++) {
				if (currentSudoku[i*N + j]->value == k + 1) {
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
	/* try to solve the currentSudoku using ILP.
		* if the board is solvable it writes the solution to the tempBoard and return 1,
		*if the board is unsolvable it doesnt change the tempBoard and return 2,
		if error occur return -1; */
	GRBmodel* model = NULL;
	GRBenv* env = NULL;
	int ThreeDMatrixSize, optimStat, flag = 0, *index;
	char* verType;
	double* lb, *val, *sol;
	ThreeDMatrixSize = N * N * N;
	lb = (double*)malloc(ThreeDMatrixSize * sizeof(double));
	verType = (char*)malloc(ThreeDMatrixSize * sizeof(char));
	index = (int*)malloc(N * sizeof(int));
	val = (double*)malloc(N * sizeof(double));
	sol = (double*)malloc(ThreeDMatrixSize * sizeof(double));
	printf("after malloc");
	addVaribles(lb, verType);
	flag = GRBloadenv(&env, "ILP.log");
	if (flag) {
		freeAll(lb, val, verType, index);
		return exitILP(&env, &model, flag, optimStat, sol, ThreeDMatrixSize);
	}
	printf("after GRBloadenv");
	flag = GRBsetintparam(env, GRB_INT_PAR_LOGTOCONSOLE, 0);
	if (flag) {
		freeAll(lb, val, verType, index);
		return exitILP(&env, &model, flag, optimStat, sol, ThreeDMatrixSize);
	}
	printf("after GRBsetintparam");
	flag = GRBnewmodel(env, &model, "ILP", ThreeDMatrixSize, NULL, lb, NULL, verType, NULL);
	if (flag) {
		freeAll(lb, val, verType, index);
		return exitILP(&env, &model, flag, optimStat, sol, ThreeDMatrixSize);
	}
	printf("after GRBnewmodel");
	flag = oneValPerCellCon(&model, index, val);
	if (flag) {
		freeAll(lb, val, verType, index);
		return exitILP(&env, &model, flag, optimStat, sol, ThreeDMatrixSize);
	}
	printf("after oneValPerCellCon");

	flag = oneValPerColCon(&model, index, val);
	if (flag) {
		freeAll(lb, val, verType, index);
		return exitILP(&env, &model, flag, optimStat, sol, ThreeDMatrixSize);
	}
	printf("after oneValPerColCon");

	flag = oneValPerRowCon(&model, index, val);
	if (flag) {
		freeAll(lb, val, verType, index);
		return exitILP(&env, &model, flag, optimStat, sol, ThreeDMatrixSize);
	}
	printf("after oneValPerRowCon");

	flag = oneValPerBlockCon(&model, index, val);
	if (flag) {
		freeAll(lb, val, verType, index);
		return exitILP(&env, &model, flag, optimStat, sol, ThreeDMatrixSize);
	}
	printf("after oneValPerBlockCon");

	flag = GRBoptimize(model);
	if (flag) {
		freeAll(lb, val, verType, index);
		return exitILP(&env, &model, flag, optimStat, sol, ThreeDMatrixSize);
	}
	printf("after GRBoptimize");

	flag = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimStat);
	if (flag) {
		freeAll(lb, val, verType, index);
		return exitILP(&env, &model, flag, optimStat, sol, ThreeDMatrixSize);
	}
	printf("after GRBgetintattr");

	freeAll(lb, val, verType, index);
	return exitILP(&env, &model, flag, optimStat, sol, ThreeDMatrixSize);
	printf("the end");

}


/*
int ILPSolver() {
	printf("bla: %d", N);
	return -1;
}*/