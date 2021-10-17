#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

// simply redirects a descriptor to another
// and closes it
static int io_redirect(int from, int to) {
  int fd = dup2(from, to);

  if (fd == -1) {
    perror("dup2");
    return -1;
  }

  if (close(from)) {
    perror("close");
    return -1;
  }

  return fd;
}

// opens a file name using flags and redirects to a
// file descriptor
static int io_redirect_name(const char *file, int to, int flags) {

  int fd = -1;

  if (file == NULL) {
    return -1;
  }

  // mode only relevant on create
  fd = open(file, flags, 0644);

  if (fd == -1) {
    perror("open");
    return -1;
  }

  return io_redirect(fd, to);
}

int io_redirect_in(int fd) {
  return io_redirect(fd, STDIN_FILENO);
}

int io_redirect_out(int fd) {
  return io_redirect(fd, STDOUT_FILENO);
}

int io_redirect_name_in(const char *file) {
  return io_redirect_name(file, STDIN_FILENO, O_RDONLY);
}

int io_redirect_name_out(const char *file) {
  return io_redirect_name(file, STDOUT_FILENO, O_WRONLY|O_CREAT|O_TRUNC);
}


