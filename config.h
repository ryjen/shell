#ifndef RYJEN_CONFIG_H
#define RYJEN_CONFIG_H

typedef struct config Config;

/** Memory allocation **/

Config *config_new();

void config_delete(Config *conf);


/** Properties **/

int config_set_shared(Config *conf, int value);

int config_get_shared(Config *conf);

/** Methods **/

int config_parse(Config *conf, int argc, char *argv[]);

int config_set_defaults(Config *conf);

#endif
