#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "parser.h"
#include "command.h"
#include "io.h"

// a command parser
struct parser {
  // the list of commands (separated by pipe)
  Command *commands;
  // the user input
  char *input;
  // the saved stdin file descriptor
  int in;
  // the saved stdout file descriptor
  int out;
  // the configuration
  Config *config;
};

// allocates a new parser into memory
Parser *parser_new() {
  Parser *value = (Parser*) malloc(sizeof(Parser));
  if (value == NULL) {
    abort();
  }
  value->commands = NULL;
  value->input = NULL;
  value->in = -1;
  value->out = -1;
  value->config = NULL;
  return value;
}

// free a parser from memory
void parser_clear(Parser *parser) {
  command_delete_list(parser->commands);
  
  if (parser->input) {
    free(parser->input);
  }
  
  if (parser->in != -1) {
    close(parser->in);
  }
  
  if (parser->out != -1) {
    close(parser->out);
  }
  parser->commands = NULL;
  parser->input = NULL;
  parser->in = -1;
  parser->out = -1;
  parser->config = NULL;

}

void parser_delete(Parser *parser) {
  parser_clear(parser);

  free(parser);
}

int parser_set_config(Parser *parser, Config *config) {
  if (parser == NULL) {
    return -1;
  }
  parser->config = config;
  return 0;
}

// copies standard in/out to parser
static int parser_save_io(Parser *parser) {
  if (parser == NULL) {
    return -1;
  }

  parser->in = dup(STDIN_FILENO);

  if (parser->in == -1) {
    perror("dup");
    return -1;
  }

  parser->out = dup(STDOUT_FILENO);
  
  if (parser->out == -1) {
    perror("dup");
    return -1;
  }

  return 0;
}

// restores previously saved std in/out from parser
static int parser_restore_io(Parser *parser) {
  if (parser == NULL) {
    return -1;
  }

  if (io_redirect_in(parser->in) == -1) {
    return -1;
  }

  if (io_redirect_out(parser->out) == -1) {
    return -1;
  }

  return 0;
}

// gets input from stdin without a line ending
static size_t parser_get_input(Parser *parser) {
  size_t n = 0;

  // get the next input
  ssize_t size = getline(&parser->input, &n, stdin);

  // error check, could be EOF
  if (size == -1) {
    perror("getline");
    return -1;
  }

  // remove the line ending
  parser->input[size-1] = '\0';

  return 0;
}

// sets the parameter argument by either
// stripping a symbol from the argument or
// strtok'ing the next argument
//
// returns 0 on success, -1 on error (no arg)
static int parser_strip_symbol(char **arg) {
  char *next = NULL;

  if (arg == NULL) {
    return -1;
  }

  next = *arg;

  if (strlen(next) > 1) {
    *arg = next + 1;
    return 0;
  }

  next = strtok(NULL, " ");

  if (next == NULL) {
    return -1;
  }

  *arg = next;

  return 0;
}

// parses stdin to a list of commands separated by pipes
int parser_read(Parser *parser) {
  Command *value = NULL;
  char *arg = NULL;
  int n = 0;

  if (parser == NULL) {
    return -1;
  }

  // get the next line
  if (parser_get_input(parser)) {
    return -1;
  }

  // no input? short circuit
  if (parser->input == NULL || *parser->input == '\0') {
    puts("No input");
    return -1;
  }

  // create a new command list for the parser
  parser->commands = value = command_new();
  
  command_set_config(value, parser->config);

  // loop through arguments in the input....
  for (n = 0, arg = strtok(parser->input, " "); 
      arg && n < command_args_size(); 
      arg = strtok(NULL, " ")) {

    // check for symbols...
    switch(arg[0]) {
      case '<':
        if (parser_strip_symbol(&arg) == -1) {
          puts("No input redirect file");
          return -1;
        }
        // redirect input file
        command_set_file_input(value, arg);
        // next arg
        continue;
      case '>':
        if (parser_strip_symbol(&arg) == -1) {
          puts("No output redirect file");
          return -1;
        }
        // redirect output file
        command_set_file_output(value, arg);
        // next arg
        continue;
      case '|':
        // finalize current command
        command_set_arg(value, n, NULL);
        n = 0;
        // create new command in list 
        value = command_next(value);
        // strip arg from current one if needed
        parser_strip_symbol(&arg);
        // fall through
        break;
    }
    // assign current arg
    command_set_arg(value, n++, arg);
  }

  // finalize current arg
  command_set_arg(value, n, NULL);

  return 0;
}

// interprets the parser
int parser_interpret(Parser *parser) {

  // save the standard io
  if (parser_save_io(parser)) {
    return -1;
  }

  // interpret the command list
  if (command_interpret(parser->commands)) {
    return -1;
  }

  // restore the standard io
  if (parser_restore_io(parser)) {
    return -1;
  }

  return 0;
}

