#include "util.h"
#include <sys/stat.h>

/*
 * @TODO:(#1) Handle Config paths array memory allocation/deallocation
 * the paths array is not utilized at the moment. I am determining how I want
 * to design the system and will circle back to it once I am further along in
 * development
 */

int file_exists(const char *filename) {
  struct stat buf;
  return (stat(filename, &buf) == 0);
}
