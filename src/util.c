#include "util.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int file_exists(const char *filename) {
  struct stat buf;
  return (stat(filename, &buf) == 0);
}

char *expand_path(const char *path) {
  if (strlen(path) == 0) {
    fprintf(stderr, "[ERROR]: Cannot expand a path that is empty\n");
    return NULL;
  }

  char *full_path = (char *)malloc(sizeof(char) * 512);
  if (full_path == NULL) {
    fprintf(stderr, "[ERROR]: Failed to allocate memory for expanded path\n");
    return NULL;
  }

  if (path[0] != '~') {
    strcpy(full_path, path);
    return full_path;
  }

  char *home_path = getenv("HOME");
  if (home_path == NULL) {
    fprintf(stderr, "[ERROR]: Failed to find $HOME in list of env variables\n");
    return NULL;
  }

  snprintf(full_path, 512, "%s%s", home_path, path + 1);
  return full_path;
}

char *read_file(const char *filename) {
  char *expanded_path = expand_path(filename);
  if (expanded_path == NULL) {
    fprintf(stderr, "[ERROR]: Failed to expand path\n");
    return NULL;
  }

  struct stat stat_buf;
  int ok = stat(expanded_path, &stat_buf);
  if (ok == -1) {
    fprintf(stderr, "[ERROR]: File [%s] does not exist\n", expanded_path);
    return NULL;
  }

  char *buf = (char *)malloc(stat_buf.st_size + 1);
  if (buf == NULL) {
    fprintf(stderr, "[ERROR]: Failed to allocate memory for [%s] buffer\n",
            expanded_path);
  }

  int fd = open(expanded_path, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "[ERROR]: Failed to open file [%s]\n", expanded_path);
    free(buf);
    return NULL;
  }

  ssize_t bytes_read = read(fd, buf, stat_buf.st_size + 1);
  if (bytes_read == -1) {
    fprintf(stderr, "[ERROR]: Failed to read file [%s]\n", expanded_path);
    close(fd);
    free(buf);
    return NULL;
  }

  buf[bytes_read] = '\0';
  close(fd);
  free(expanded_path);
  return buf;
}
