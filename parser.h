#ifndef RYJEN_PARSER_H
#define RYJEN_PARSER_H

#include "config.h"

// the parser type
typedef struct parser Parser;

/** Memory Allocation **/
// constructor
Parser *parser_new();

// destructor
void parser_delete(Parser *);

/** Properties **/

// configures a parser
int parser_set_config(Parser *, Config *config);

/** Methods **/

// clears a parsers data
void parser_clear(Parser *);

// reads input from standard input
int parser_read(Parser *);

// interprets commands and executes
int parser_interpret(Parser *);

#endif

