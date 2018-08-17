#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include "Game.h"

extern Cell** currentSudoku;
extern mode;

int initNumberOfHints() {/*#######################################3delete###########################*/
	char hints[20];
	int flag = 1;
	int convertHintsToInt = -1;
	while (flag) {
		printf("Please enter the number of cells to fill [0-80]:\n");
		if (fgets(hints, 1024, stdin) == NULL) {
			exitGame();
		}
		convertHintsToInt = atoi(hints);
		if (hints[0] == '0') {
			return 0;
		}
		if (convertHintsToInt > 80 || convertHintsToInt <= 0) {
			printf("Error: invalid number of cells to fill (should be between 0 and 80)\n");
		}
		else flag = 0;
	}
	return convertHintsToInt;
}

void printInvalid() {
	printf("ERROR: invalid command\n");
}

char* getCommand() {
	char command[1024];
	int j = 1, flag = 1, check, numOfMarkError, i = 0, temp;
	char* newType;
	char* values = (char*)malloc(sizeof(char) * 256);
	idCommand = 0;
	if (values == NULL) {
		printf("Error: getCommand has failed\n");
		exit(0);
	}
	memset(values, '\0', 256);
	while (flag)
	{
		j = 1;
		printf("Enter your command:\n");
		if (fgets(command, 1024, stdin) == NULL) {
			exitGame();
		}
		newType = strtok(command, " \t\r\n");
		if ((check = strcmp(newType, "solve")) == 0) {
			idCommand = 1;
			while ((newType = strtok(NULL, "\n")) != NULL) {
				strcpy(values, newType);
			}
			flag = 0;
		}
		else if ((check = strcmp(newType, "edit")) == 0) {
			idCommand = 2;
			while ((newType = strtok(NULL, "\n")) != NULL) {
				strcpy(values, newType);
			}
			flag = 0;
		}
		else if ((check = strcmp(newType, "mark_errors")) == 0) {
			if (mode == 1) { /*This command avaliable only in solve mode (1)*/
				while (newType != NULL && j < 2) {
					newType = strtok(NULL, " \t\r\n");
					strcpy(values, newType);
					j++;
				}
				numOfMarkError = atoi(values);
				if (numOfMarkError == 0 || numOfMarkError == 1) {
					values[0] = '6'; /* 6 represent mark_error command*/
					values[1] = (numOfMarkError + '0');
					flag = 0;
				}
				else {
					printf("Error: the value should be 0 or 1\n");
				}
			}
			else {
				printInvalid();
			}
		}
		else if ((check = strcmp(newType, "set")) == 0) {
			if (mode == 1 || mode == 2) {
				while (newType != NULL && j < 4) {
					newType = strtok(NULL, " \t\r\n");
					for (i = 0; i < (int)strlen(newType); i++) {
						if ((isdigit((int)newType[i])) == 0) {
							printf("Error: value not in range 0-%d\n", N);
							return values;
						}
					}
					temp = atoi(newType);
					if ((temp < 0) || (temp > N)){
						printf("Error: value not in range 0-%d\n", N);
						return values;
					}
					else {
						values[j] = temp + '0';
						j++;
					}
				}
				values[0] = '1';
				flag = 0;
			}
			else {
				printInvalid();
			}
		}
		else if ((check = strcmp(newType, "generate")) == 0) {
			if (mode == 2) {
				while (newType != NULL && j < 4) {
					newType = strtok(NULL, " \t\r\n");
					for (i = 0; i < (int)strlen(newType); i++) {
						numOfEmptyCells = checkNumOfEmptyCells(currentSudoku);
						if ((isdigit((int)newType[i])) == 0) {
							printf("Error: value not in range 1-%d\n", numOfEmptyCells);
							return values;
						}
					}
					temp = atoi(newType);
					if ((temp < 0) || (temp > numOfEmptyCells)) {
						printf("Error: value not in range 1-%d\n", numOfEmptyCells);
						return values;
					}
					else {
						values[j] = temp + '0';
						j++;
					}
				}
				values[0] = 'c';
				flag = 0;
			}
			else {
				printInvalid();
			}
		}
		else if ((check = strcmp(newType, "print_board")) == 0) {
			if (mode == 1 || mode == 2) {
				values[0] = '7';
				flag = 0;
			}
			else printInvalid();
			
		}
		else if ((check = strcmp(newType, "hint")) == 0) {
			printf("mode=%d", mode);
			if (mode == 1) {
				while (newType != NULL && j < 3) {
					newType = strtok(NULL, " \t\r\n");
					for (i = 0; i < (int)strlen(newType); i++) {
						if ((isdigit((int)newType[i])) == 0) {
							printf("Error: value not in range 1-%d\n", N);
							return values;
						}
					}
					temp = atoi(newType);
					if ((temp < 1) || (temp > N)) {
						printf("Error: value not in range 1-%d\n", N);
						return values;
					}
					else {
						values[j] = temp + '0';
						j++;
					}
				}
				values[0] = '2';
				flag = 0;
				printf("after flag=0");
			}
			else {
				printInvalid();
			}
		}
		else if ((check = strcmp(newType, "validate")) == 0) {
			if (mode == 1 || mode == 2) {
				values[0] = '3';
				flag = 0;
			}
			else printInvalid();
		}
		else if ((check = strcmp(newType, "autofill")) == 0) {
			if (mode == 1) {
				values[0] = '8';
				flag = 0;
			}
			else printInvalid();
		}
		else if ((check = strcmp(newType, "save")) == 0) {
			if (mode == 1 || mode == 2) {
				idCommand = 3;
				while ((newType = strtok(NULL, "\n")) != NULL) {
					strcpy(values, newType);
				}
				flag = 0;
			}
			else printInvalid();
		}
		else if ((check = strcmp(newType, "undo")) == 0) {
			if (mode == 1 || mode == 2) {
				values[0] = 'b';
				flag = 0;
			}
			else printInvalid();
		}
		else if ((check = strcmp(newType, "redo")) == 0) {
			if (mode == 1 || mode == 2) {
				values[0] = 'a';
				flag = 0;
			}
			else printInvalid();
		}
		else if ((check = strcmp(newType, "num_solutions")) == 0) {
			if (mode == 1 || mode==2) {
				values[0] = '9';
				flag = 0;
			}
			else printInvalid();
		}
		else if ((check = strcmp(newType, "reset")) == 0) {
			if (mode == 1 || mode == 2) {
				values[0] = '4';
				flag = 0;
			}
			else printInvalid();
		}
		else if ((check = strcmp(newType, "exit")) == 0) {
			if (mode == 1 || mode == 2) {
				values[0] = '5';
				flag = 0;
			}
		}
		else printInvalid();
	}
	return values;

}





