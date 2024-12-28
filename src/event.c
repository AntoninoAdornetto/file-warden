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

  state->wd_entry_count = 0;
  for (int i = 0; i < cfg->paths_size; i++) {
    if (i > MAX_WATCH_DESCRIPTORS - 1) {
      syslog(LOG_ERR, "Cannot watch more than [%d] files/directories",
             MAX_WATCH_DESCRIPTORS);
      stop_event_listener(state);
      return NULL;
    }

    // @TODO: utilize configuration events instead of hardcoding the event mask
    state->wd[i] = inotify_add_watch(state->fd, cfg->paths[i], IN_ACCESS);
    if (state->wd[i] == -1) {
      syslog(LOG_ERR, "Failed to add [%s] to inotify watch list. [error: %s]",
             cfg->paths[i], strerror(errno));
      stop_event_listener(state);
      return NULL;
    }

    if (cfg->paths[i] == NULL) {
      syslog(LOG_ERR,
             "Expected index [%d] to contain a valid path but got null", i);
      stop_event_listener(state);
      return NULL;
    }

    state->wd_map[i].path = malloc(sizeof(char) * strlen(cfg->paths[i]) + 1);
    if (state->wd_map[i].path == NULL) {
      syslog(LOG_ERR,
             "Failed to allocate memory for path mapping at index [%d]", i);
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

  syslog(LOG_INFO, "File event listener has started...");
  return state;
}

void stop_event_listener(EventState *state) {
  if (state == NULL) {
    return;
  }

  for (int i = 0; i < state->wd_entry_count; i++) {
    int status = inotify_rm_watch(state->fd, state->wd[i]);
    if (status == -1) {
      syslog(LOG_ERR, "Failed to remove watch descriptor from inotify event");
    }
    free(state->wd_map[i].path);
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

char *get_wd_path_mapping(EventState *state, int wd) {
  for (int i = 0; i < state->wd_entry_count; i++) {
    if (state->wd_map[i].wd == wd) {
      return state->wd_map[i].path;
    }
  }

  return NULL;
}

/*
 * @TODO:(#8) event handler should use cfg option settings for notifications
 * Currently, the file system events that are listened to, by inotify, are
 * hardcoded. This was for testing purposes so that I could acclimate myself to
 * the inotify & libnotify APIs. Ideally, we should use the event mask, read in
 * from our programs settings instead of hardcoding. Additionally, we should
 * adjust the `display_notification` function so that it can prepare a
 * notification message that also utilizes the events mask read in
 * from the programs setting. This can be done in an idiomatic fashion. Lastly,
 * the system logs to `journal` can be moved into the `display_notification`
 * function.
 */
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

      char *path = get_wd_path_mapping(state, event->wd);
      if (path == NULL) {
        syslog(LOG_ERR, "Failed to retrieve wd -> path mapping");
        return -1;
      }

      if (event->mask & IN_ACCESS) {
        int status = display_notification(path, "Was accessed");
        if (status != 0) {
          syslog(LOG_WARNING, "Failed to display file system access event. "
                              "Note: event logged to journal");
        }

        syslog(LOG_INFO, "%s was accessed!", path);
      }

      if (event->mask & IN_MODIFY) {
        int status = display_notification(path, "Was modified");
        if (status != 0) {
          syslog(LOG_WARNING, "Failed to display 'modify' file system event. "
                              "Note: event logged to journal");
        }
        syslog(LOG_INFO, "%s was modifed!", path);
      }
    }
  }

  return 0;
}
