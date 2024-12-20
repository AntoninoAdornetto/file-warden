#include "util.h"
#include <sys/stat.h>

int file_exists(const char *filename) {
  struct stat buf;
  return (stat(filename, &buf) == 0);
}
