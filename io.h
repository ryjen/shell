#ifndef RYJEN_IO_H
#define RYJEN_IO_H

// redirect a file descriptor to stdin
// returns the new descriptor or -1 on error
int io_redirect_in(int);

// redirect a file descriptor to stdout
// returns the new descriptor or -1 on error
int io_redirect_out(int);

// redirect a file name to stdin
// returns the new descriptor or -1 on error
int io_redirect_name_in(const char *);

// redirect a file name to stdout
// returns the new descriptor or -1 on error
int io_redirect_name_out(const char *);

#endif

