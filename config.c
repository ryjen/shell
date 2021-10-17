#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>

#include "config.h"

struct config {
  int shared;
};

Config *config_new() {
  Config *value = (Config*) malloc(sizeof(Config));

  value->shared = 0;

  return value;
}

void config_delete(Config *config) {
  if (config == NULL) {
    return;
  }
  free(config);
}

int config_set_shared(Config *config, int value) {
  if (config == NULL) {
    return -1;
  }
  config->shared = value > 0 ? 1 : 0;
  return 0;
}

int config_get_shared(Config *config) {
  if (config == NULL) {
    return -1;
  }
  return config->shared;
}

int config_set_defaults(Config *config) {
  if (config == NULL) {
    return -1;
  }
  config->shared = 0;
  return 0;
}

int config_parse(Config *config, int argc, char *argv[]) {
  int opt = 0;
  int c = -1;
  static const struct option options[] = {
    { "shared",   no_argument,  0,  's' },
    { "help",     no_argument,  0,  'h' },
    { 0,  0,  0,  0}
  };

  while((c = getopt_long(argc, argv, "sh", options, &opt)) != -1)  {
    switch(c) {
      default:
        printf("Syntax: %s ", argv[0]);
        for (int i = 0; options[i].name; i++) {
          printf("--%s ", options[i].name);
        }
        printf("\n");
        return 1;
      case 's':
        config->shared = 1;
        break;
    }
  }
  return 0;
}

