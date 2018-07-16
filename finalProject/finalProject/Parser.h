/*
This module reads and interprets the user inputs.
*/

#ifndef PARSER_H_
#define PARSER_H_

/*This function get from the user the number of hints that he wants to start with in his sudoku game
and interpret this number.
@return
the number of hints - should be between 1 to 80
*/
int initNumberOfHints();

/**This function get the command that the user write in the command line and interpret
to one of our possibles command:
1->set, 2->hint, 3->validate, 4->restart, 5->exit
@return
values - array of chars that:
values[0]=: 1->set, 2->hint, 3->validate, 4->reset, 5->exit, 6->mark_error, 7->print_board ,8->autofill, 9->num_solutions
10->undo, 11->redo, 12->generate
If its set or hint command the values of the row, col and the number will be in the array
*/
char* getCommand();

#endif /*PARSERS_H_*/
