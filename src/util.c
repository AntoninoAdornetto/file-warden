#include "util.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>

int file_exists(const char *filename) {
  struct stat buf;
  int exists = stat(filename, &buf);
  return exists == 0;
}

char *expand_path(const char *path) {
  if (strlen(path) == 0) {
    syslog(LOG_ERR, "Cannot expand a path that is empty\n");
    return NULL;
  }

  char *full_path = (char *)malloc(sizeof(char) * 512);
  if (full_path == NULL) {
    syslog(LOG_ERR, "Failed to allocate memory for expanded path\n");
    return NULL;
  }

  if (path[0] != '~') {
    strcpy(full_path, path);
    return full_path;
  }

  char *home_path = getenv("HOME");
  if (home_path == NULL) {
    syslog(LOG_ERR, "Failed to find $HOME in list of env variables\n");
    return NULL;
  }

  snprintf(full_path, 512, "%s%s", home_path, path + 1);
  return full_path;
}

char *read_file(const char *filename) {
  struct stat stat_buf;
  int ok = stat(filename, &stat_buf);
  if (ok == -1) {
    syslog(LOG_ERR, "File [%s] does not exist\n", filename);
    return NULL;
  }

  char *buf = (char *)malloc(stat_buf.st_size + 1);
  if (buf == NULL) {
    syslog(LOG_ERR, "Failed to allocate memory for [%s] buffer\n", filename);
    return NULL;
  }

  int fd = open(filename, O_RDONLY);
  if (fd == -1) {
    syslog(LOG_ERR, "Failed to open file [%s]\n", filename);
    free(buf);
    return NULL;
  }

  ssize_t bytes_read = read(fd, buf, stat_buf.st_size + 1);
  if (bytes_read == -1) {
    syslog(LOG_ERR, "Failed to read file [%s]\n", filename);
    close(fd);
    free(buf);
    return NULL;
  }

  buf[bytes_read] = '\0';
  close(fd);
  return buf;
}

int init_signals(struct sigaction *sa) {
  const SignalMapping sigs[] = {
      {SIGTERM},
      {SIGHUP},
      {SIGINT},
      {-1},
  };

  for (int i = 0; sigs[i].signal != -1; i++) {
    if (sigaction(sigs[i].signal, sa, NULL) != 0) {
      return EXT_INIT_SIGNALS;
    }
  }

  return 0;
}
