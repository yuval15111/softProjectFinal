#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include "Game.h"

int initNumberOfHints() {
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


char* getCommand() {
	char command[1024];
	int j = 1, flag = 1, check, numOfMarkError;
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
				printf("ERROR: invalid command\n");
			}

		}
		else if ((check = strcmp(newType, "set")) == 0) {
			if (mode == 1 || mode == 2) {
				while (newType != NULL && j < 4) {
					newType = strtok(NULL, " \t\r\n");
					values[j] = *newType;
					j++;
				}
				printf("set:x= %c, y=%c, z=%c\n", values[1], values[2], values[3]);
				if (((values[1] - '0') < 0 || (values[1] - '0') > N) || ((values[2] - '0') < 0 || (values[2] - '0') > N)
					|| ((values[3] - '0') < 0 || (values[3] - '0') > N)) {

				}
				else {
					values[0] = '1';
					flag = 0;
				}
			}
			else {
				printf("ERROR: invalid command\n");
			}
		}
		else if ((check = strcmp(newType, "hint")) == 0) {
			while (newType != NULL && j < 3) {
				newType = strtok(NULL, " \t\r\n");
				values[j] = *newType;
				j++;
			}
			values[0] = '2';
			flag = 0;
		}
		else if ((check = strcmp(newType, "validate")) == 0) {
			values[0] = '3';
			flag = 0;
		}
		else if ((check = strcmp(newType, "restart")) == 0) {
			values[0] = '4';
			flag = 0;
		}
		else if ((check = strcmp(newType, "exit")) == 0) {
			values[0] = '5';
			flag = 0;
		}
		else printf("ERROR: invalid command\n");
	}
	return values;
}





