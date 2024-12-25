#include "event.h"
#include "config.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/syslog.h>
#include <unistd.h>

EventState *start_event_listener(Config *cfg) {
  EventState *state = malloc(sizeof(EventState));
  if (state == NULL) {
    syslog(LOG_ERR, "Failed to allocate memory for inotify event state\n");
    return NULL;
  }

  state->fd = inotify_init1(IN_NONBLOCK);
  if (state->fd == -1) {
    syslog(LOG_ERR, "Failed to create and initialize inotify instance\n");
    stop_event_listener(state);
    return NULL;
  }

  state->wd = calloc(cfg->paths_size, sizeof(int));
  if (state->wd == NULL) {
    syslog(LOG_ERR,
           "Failed to allocate memory for inotify watch descriptors\n");
    stop_event_listener(state);
    return NULL;
  }

  state->wd_entry_count = 0;
  for (int i = 0; i < cfg->paths_size; i++) {
    if (i > MAX_WATCH_DESCRIPTORS - 1) {
      syslog(LOG_ERR, "Cannot watch more than [%d] files/directories\n",
             MAX_WATCH_DESCRIPTORS);
      stop_event_listener(state);
      return NULL;
    }

    // @TODO: utilize configuration events instead of hardcoding the event mask
    state->wd[i] = inotify_add_watch(state->fd, cfg->paths[i], IN_ACCESS);
    if (state->wd[i] == -1) {
      syslog(LOG_ERR, "Failed to add [%s] to inotify watch list. [error: %s]\n",
             cfg->paths[i], strerror(errno));
      stop_event_listener(state);
      return NULL;
    }

    if (cfg->paths[i] == NULL) {
      syslog(LOG_ERR,
             "Expected index [%d] to contain a valid path but got null\n", i);
      stop_event_listener(state);
      return NULL;
    }

    state->wd_map[i].path = malloc(sizeof(char) * strlen(cfg->paths[i]) + 1);
    if (state->wd_map[i].path == NULL) {
      syslog(LOG_ERR,
             "Failed to allocate memory for path mapping at index [%d]\n", i);
      stop_event_listener(state);
      return NULL;
    }

    state->wd_map[i].wd = state->wd[i];
    strcpy(state->wd_map[i].path, cfg->paths[i]);
    state->wd_entry_count++;
  }

  state->nfds = 1;
  state->fds[0].fd = state->fd;
  state->fds[0].events = POLLIN;

  syslog(LOG_INFO, "File event listener has started...\n");
  return state;
}

void stop_event_listener(EventState *state) {
  if (state == NULL) {
    return;
  }

  if (state->fd != -1) {
    close(state->fd);
    syslog(LOG_INFO, "File event listener has stopped...\n");
  }

  for (int i = 0; i < state->wd_entry_count; i++) {
    free(state->wd_map[i].path);
  }

  if (state->wd != NULL) {
    free(state->wd);
  }

  free(state);
}
