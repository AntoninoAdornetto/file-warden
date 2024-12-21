#ifndef UTIL_H
#define UTIL_H
#include <stdint.h>

#define EXT_INVAL_PATH

typedef uint8_t u8;

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
 * Convenience function for expanding paths. eg. "~/.ssh" -> "$HOME/.ssh/"
 */
char *expand_path(const char *path);

#endif
