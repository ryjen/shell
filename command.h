#ifndef RYJEN_COMMAND_H
#define RYJEN_COMMAND_H

#include "config.h"

// the type definition for a command
typedef struct command Command;

/** Memory Allocation **/

// constructor
Command *command_new();

// destructor
void command_delete(Command *);

// deletes a list of commands
void command_delete_list(Command *);

/** Properties **/

// sets the config for a command
int command_set_config(Command *, Config *);

// get the size of the command arguments
int command_args_size();

// set an argument in the command
int command_set_arg(Command *, int, char *);

// set file input redirection
int command_set_file_input(Command *, const char *);

// set file output redirection
int command_set_file_output(Command *, const char *);

/** Methods **/

// add a new command to an existing one
Command *command_next(Command *);

// interprets and executes a list of commands
int command_interpret(Command *);

#endif

