/*
This module reads and interprets the user inputs.
*/

#ifndef PARSER_H_
#define PARSER_H_

/*
This function prints an error.
*/
void printInvalid();

/**This function gets the command that the user wrote in the command line and interpret
to one of the possibillities below:
idCommand=1 ->solve, idCommand=2 ->edit, idCommand=3 ->save, 1 ->set, 2 ->hint, 
3 ->validate, 4 ->reset, 5 ->exit, 6 ->mark_errors, 7 ->print_board, 
8 ->autofill, 9 ->num_solutions, a ->redo, b ->undo, c ->generate,
@return
values - array of chars that:
values[0]:= the char that we wrote above for every command.
if the command has i arguments (numbers), it will be saves in values[1],...,values[i].
*/
char* getCommand();

#endif /*PARSERS_H_*/
