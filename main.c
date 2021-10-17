#include <stdio.h>
#include "parser.h"
#include "config.h"

int main(int argc, char *argv[]) {

  Config *config = config_new();

  Parser *parser = parser_new();

  config_set_defaults(config);

  config_parse(config, argc, argv);

  parser_set_config(parser, config);

  // keep running, use ctrl-c to quit
  while(1) {

    // print prompt
    printf("> ");

    // read command from standard input
    if (parser_read(parser)) {
      return 1;
    }

    // interpret the commands and run
    if (parser_interpret(parser)) {
      return 1;
    }

    // cleanup
    parser_clear(parser);
  }

  return 0;
}


