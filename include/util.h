#ifndef UTIL_H
#define UTIL_H

#include <signal.h>
#include <stdint.h>

#define EXT_INIT_SIGNALS 8

typedef uint8_t u8;

typedef struct {
  int signal;
} SignalMapping;

/*
 * Convenience function to simply check if a file exists
 */
int file_exists(const char *filename);

/*
 * Convenience function for reading a file and returning
 * heap allocated string. YOU are responsible for freeing
 * the memory afterwards.
 */
char *read_file(const char *filename);

/*
 * Wrapper function for invoking [sigaction] with all signals we are concered
 * with for cleanly exiting the systemd service. On error, the function will
 * return a non-zero integer.
 */
int init_signals(struct sigaction *sa);

/*
 * Expands input [path]. NOTE: as of now, it only expands the '~' -> $HOME env.
 */
char *expand_path(const char *path);

#endif
