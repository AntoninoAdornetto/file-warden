#include "event.h"
#include "config.h"
#include "notify.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/syslog.h>
#include <unistd.h>

EventState *start_event_listener(Config *cfg) {
  EventState *state = malloc(sizeof(EventState));
  if (state == NULL) {
    syslog(LOG_ERR, "Failed to allocate memory for inotify event state");
    return NULL;
  }

  state->fd = inotify_init1(IN_NONBLOCK);
  if (state->fd == -1) {
    syslog(LOG_ERR, "Failed to create and initialize inotify instance");
    stop_event_listener(state);
    return NULL;
  }

  state->wd = calloc(cfg->paths_size, sizeof(int));
  if (state->wd == NULL) {
    syslog(LOG_ERR, "Failed to allocate memory for inotify watch descriptors");
    stop_event_listener(state);
    return NULL;
  }

  for (int i = 0; i < cfg->paths_size; i++) {
    if (i > MAX_WATCH_DESCRIPTORS - 1) {
      syslog(LOG_ERR, "Cannot exceed [%d] wd's", MAX_WATCH_DESCRIPTORS);
      stop_event_listener(state);
      return NULL;
    }

    if (cfg->paths[i] == NULL) {
      syslog(LOG_ERR, "Expected index [%d] to contain a watch path", i);
      stop_event_listener(state);
      return NULL;
    }

    state->wd[i] =
        inotify_add_watch(state->fd, cfg->paths[i], cfg->events_mask);

    if (state->wd[i] == -1) {
      syslog(LOG_ERR, "Failed to add [%s] to inotify watch list. [error: %s]",
             cfg->paths[i], strerror(errno));
      stop_event_listener(state);
      return NULL;
    }
  }

  state->nfds = 1;
  state->fds[0].fd = state->fd;
  state->fds[0].events = POLLIN;

  syslog(LOG_INFO, "File event listener has started...");
  return state;
}

void stop_event_listener(EventState *state) {
  if (state == NULL) {
    return;
  }

  for (int i = 0; state->wd[i]; i++) {
    int status = inotify_rm_watch(state->fd, state->wd[i]);
    if (status == -1) {
      syslog(LOG_ERR, "Failed to remove watch descriptor from inotify event");
    }
  }

  if (state->fd != -1) {
    close(state->fd);
    syslog(LOG_INFO, "File event listener has stopped...");
  }

  if (state->wd != NULL) {
    free(state->wd);
  }

  free(state);
}

int handle_events(EventState *state) {
  char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
  const struct inotify_event *event;
  size_t len;

  for (;;) {
    syslog(LOG_INFO, "Reading file descriptor [%d]", state->fd);
    len = read(state->fd, buf, sizeof(buf));

    if (len == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      }

      syslog(LOG_ERR, "Failed to read events (fd=%d): %s", state->fd,
             strerror(errno));
      return -1;
    }

    if (len == 0) {
      break;
    }

    for (char *ptr = buf; ptr < buf + len;
         ptr += sizeof(struct inotify_event) + event->len) {
      event = (const struct inotify_event *)ptr;

      int status = notify(event);
      if (status != 0) {
        syslog(LOG_WARNING, "Failed to send desktop notification");
        syslog(LOG_WARNING, "Event may have been logged to journal");
      }
    }
  }

  return 0;
}
