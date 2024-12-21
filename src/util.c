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
  char *cleaned_path = expand_path(filename);
  if (cleaned_path == NULL) {
    fprintf(stderr, "[ERROR]: Failed to expand path\n");
    return NULL;
  }

  struct stat stat_buf;
  int ok = stat(cleaned_path, &stat_buf);
  if (ok == -1) {
    fprintf(stderr, "[ERROR]: File [%s] does not exist\n", cleaned_path);
    return NULL;
  }

  char *buf = (char *)malloc(stat_buf.st_size + 1);
  if (buf == NULL) {
    fprintf(stderr, "[ERROR]: Failed to allocate memory for [%s] buffer\n",
            cleaned_path);
  }

  int fd = open(cleaned_path, O_RDONLY);
  if (fd == -1) {
    fprintf(stderr, "[ERROR]: Failed to open file [%s]\n", cleaned_path);
    free(buf);
    return NULL;
  }

  ssize_t bytes_read = read(fd, buf, stat_buf.st_size + 1);
  if (bytes_read == -1) {
    fprintf(stderr, "[ERROR]: Failed to read file [%s]\n", cleaned_path);
    close(fd);
    free(buf);
    return NULL;
  }

  buf[bytes_read] = '\0';
  close(fd);
  free(cleaned_path);
  return buf;
}
