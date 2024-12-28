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

  for (int i = 0; i < state->wd_entry_count; i++) {
    int status = inotify_rm_watch(state->fd, state->wd[i]);
    if (status == -1) {
      syslog(LOG_ERR, "Failed to remove watch descriptor from inotify event\n");
    }
    free(state->wd_map[i].path);
  }

  if (state->fd != -1) {
    close(state->fd);
    syslog(LOG_INFO, "File event listener has stopped...\n");
  }

  if (state->wd != NULL) {
    free(state->wd);
  }

  free(state);
}

char *get_wd_path_mapping(EventState *state, int wd) {
  for (int i = 0; i < state->wd_entry_count; i++) {
    if (state->wd_map[i].wd == wd) {
      return state->wd_map[i].path;
    }
  }

  return NULL;
}

int handle_events(EventState *state) {
  char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
  const struct inotify_event *event;
  size_t len;

  for (;;) {
    syslog(LOG_INFO, "Reading file descriptor [%d]\n", state->fd);
    len = read(state->fd, buf, sizeof(buf));

    if (len == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      }

      syslog(LOG_ERR, "Failed to read events (fd=%d): %s\n", state->fd,
             strerror(errno));
      return -1;
    }

    if (len == 0) {
      break;
    }

    for (char *ptr = buf; ptr < buf + len;
         ptr += sizeof(struct inotify_event) + event->len) {
      event = (const struct inotify_event *)ptr;

      char *path = get_wd_path_mapping(state, event->wd);
      if (path == NULL) {
        syslog(LOG_ERR, "Failed to retrieve wd -> path mapping\n");
        return -1;
      }

      if (event->mask & IN_ACCESS) {
        syslog(LOG_INFO, "%s was accessed!\n", path);
      }

      if (event->mask & IN_MODIFY) {
        syslog(LOG_INFO, "%s was modifed!\n", path);
      }
    }
  }

  return 0;
}
