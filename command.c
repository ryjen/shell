#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sched.h>
#include <errno.h>

#include "command.h"
#include "io.h"

#define MAXARGS 1024

// pipe defines
#define INPIPE 0
#define OUTPIPE 1
#define MAXPIPE 2

// a named file descriptor
struct fileio { 
  const char *name;
  int descr;
};

// representing a command in between pipes
struct command {
  struct command *next;
  // arguments for the command
  char *args[MAXARGS];
  // the input (stdin, pipe or file)
  struct fileio in;
  // the output (stdout, pipe or file)
  struct fileio out;
  // the configuration for execution
  Config *config;
};

#if 0
static int __command_secure(Config *config) {
  if (unshare(CLONE_FILES|CLONE_FS|CLONE_NEWCGROUP|CLONE_NEWIPC|CLONE_NEWNET|CLONE_NEWNS|
        CLONE_NEWPID|CLONE_NEWUTS|CLONE_SYSVSEM) == 0) {
    return 0;
  }

  if (errno == EPERM) {

    if (config_get_shared(config) == 1) {
      return 0;
    }

    // TODO: gain permission
    puts("Run with the --shared flag to opt out of the isolation chamber");
    return -1;
  } 

  perror("unshare");
  return -1;
}
#endif


int command_args_size() {
  return MAXARGS-1;
}

// allocate and initialize a new command into memory
Command *command_new() {
  Command *value = (Command*) malloc(sizeof(Command));
  if (value == NULL) {
    abort();
  }
  memset(value->args, 0, sizeof(value->args));
  value->in.name = NULL;
  value->in.descr = -1;
  value->out.name = NULL;
  value->out.descr = -1;
  value->next = NULL;
  value->config = NULL;
  return value;
}

Command *command_next(Command *value) {
  value->next = command_new();
  // share configuration
  value->next->config = value->config;
  return value->next;
}

void command_delete(Command *value) {
  value->next = NULL;
  if (value->in.descr != -1) {
    close(value->in.descr);
  }
  if (value->out.descr != -1) {
    close(value->out.descr);
  }
  free(value);
}

// frees a command list from memory
void command_delete_list(Command *cmd) {
  while(cmd) {
    Command *next = cmd->next;
    command_delete(cmd);
    cmd = next;
  }
}

int command_set_config(Command *cmd, Config *config) {
  if (cmd == NULL) {
    return -1;
  }
  cmd->config = config;
  return 0;
}

int command_set_file_input(Command *cmd, const char *file) {
  if (cmd == NULL) {
    return -1;
  }
  cmd->in.name = file;
  return 0;
}

int command_set_file_output(Command *cmd, const char *file) {
  if (cmd == NULL) {
    return -1;
  }
  cmd->out.name = file;
  return 0;
}

int command_set_arg(Command *cmd, int index, char *arg) {
  if (cmd == NULL || index < 0 || index > MAXARGS) {
    return -1;
  }

  cmd->args[index] = arg;
  return 0;
}

// executes a command honoring redirects
static int command_execute(Command *cmd) {

  // redirect stdin as needed
  if (cmd->in.name) {
    cmd->in.descr = io_redirect_name_in(cmd->in.name);

    if (cmd->in.descr == -1) {
      return -1;
    }
  }

  // redirect stdout as needed
  if (cmd->out.name) {
    cmd->out.descr = io_redirect_name_out(cmd->out.name);

    if (cmd->out.descr == -1) {
      return -1;
    }
  }

  // execute the command
  if (execvp(cmd->args[0], cmd->args) == -1) {
    printf("Unable to execute '%s'\n", cmd->args[0]);
    return -1;
  }

  return -1;
}

// forks and executes a new command.  will handle redirection.
static int command_fork(Command *cmd) {

  if (cmd == NULL) {
    return 1;
  }

  // fork a child for command
  pid_t pid = fork();

  if (pid == -1) {
    perror("fork");
    return -1;
  }

  // if parent...
  if (pid != 0) {

    // wait for the child
    if (waitpid(pid, NULL, 0) == -1) {
      perror("waitpid");
      return -1;
    }

    return 0;
  }

  // child executes
  return command_execute(cmd);
}

// creates a pipe and fork-executes two commands
static int command_pipe(Command *writer, Command *reader) {
  int fds[MAXPIPE] = {-1,-1};
  pid_t pid1 = -1, pid2 = -1;

  if (pipe(fds) == -1) {
    perror("pipe");
    return -1;
  }

  // fork the writer
  pid1 = fork();
  switch(pid1) {
    case -1:
      perror("fork");
      return -1;

    case 0:
      // close input
      close(fds[INPIPE]);

      // redirect pipe to standard out
      if (io_redirect_out(fds[OUTPIPE]) == -1) {
        return -1;
      }

      // execute the writing process
      return command_execute(writer);

    default:
      break;
  }

  pid2 = fork();
  switch(pid2) {
    case -1:
      perror("fork");
      return -1;

    case 0:
      close(fds[OUTPIPE]);

      // redirect pipe to stdin
      if (io_redirect_in(fds[INPIPE]) == -1) {
        return -1;
      }

      // interpret the rest of the commands
      return command_interpret(reader);

    default:
      break;
  }

  // not needed in parent any more
  close(fds[INPIPE]);
  close(fds[OUTPIPE]);

  // wait for the children, no zombies
  if (waitpid(pid1, NULL, 0) == -1
      || waitpid(pid2, NULL, 0) == -1) {
    perror("waitpid");
    return -1;
  }

  return 0;
}

// execute a command
int command_interpret(Command *cmd) {

  if (cmd == NULL) {
    return 1;
  }

  // no pipes?
  if (cmd->next == NULL) {
    // just fork and execute
    return command_fork(cmd);
  }

  // while two commands...
  while(cmd && cmd->next) {
    // pipe them
    if (command_pipe(cmd, cmd->next)) {
      return 1;
    }

    // get next pair
    cmd = cmd->next->next;
  }

  // if odd number of pipes...
  if (cmd) {
    // execute the last
    if (command_fork(cmd)) {
      return 1;
    }
  }

  return 0;
}

