#include "notify.h"
#include "config.h"
#include <libnotify/notification.h>
#include <libnotify/notify.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

LibNotifyState *init_libnotify(Config *cfg) {
  if (cfg->paths_size <= 0) {
    fprintf(stderr, "[ERROR]: Must specify at least one patch to watch\n");
    return NULL;
  }

  LibNotifyState *state = malloc(sizeof(LibNotifyState));
  if (state == NULL) {
    fprintf(stderr, "[ERROR]: Failed to allocate memory for libnotify state\n");
    return NULL;
  }

  state->event_queue = -1;
  state->event_status = -1;

  bool status = notify_init("file-warden");
  if (!status) {
    fprintf(stderr, "[ERROR]: Failed to initialize lib notify\n");
    free(state);
    return NULL;
  }

  state->event_queue = inotify_init();
  if (state->event_queue == -1) {
    fprintf(stderr, "[ERROR]: Failed to initialize inotify instance\n");
    free(state);
    return NULL;
  }

  u32 mask = create_libnotify_mask(cfg->events_bmask);
  state->event_status =
      inotify_add_watch(state->event_queue, cfg->paths[0], mask);
  if (state->event_status == -1) {
    fprintf(stderr, "[ERROR]: Failed adding file to watch instance\n");
    free(state);
    return NULL;
  }

  state->msg = NULL;
  return state;
}

u32 create_libnotify_mask(u32 flags) {
  const LibNotifyEventMapping events[] = {
      {FLAG_ACCESS, IN_ACCESS},
      {FLAG_MODIFY, IN_MODIFY},
      {FLAG_CLOSE, (IN_CLOSE | IN_CLOSE_WRITE)},
      {FLAG_MOVE, (IN_MOVE | IN_MOVE_SELF)},
      {0, 0},
  };

  u32 mask = 0;
  for (int i = 0; events[i].event_mask != 0; i++) {
    mask |= events[i].libnotify_mask;
  }

  return mask;
}

int watch_libnotify(LibNotifyState *state, char *path) {
  state->read_len = read(state->event_queue, state->buf, sizeof(state->buf));
  if (state->read_len == -1) {
    fprintf(stderr, "[ERROR]: Failed to read inotify instance\n");
    return EXT_ERR_READ_INOTIFY;
  }

  for (char *buff_ptr = state->buf; buff_ptr < state->buf + state->read_len;
       buff_ptr += sizeof(struct inotify_event) + state->event->len) {
    state->msg = NULL;
    state->event = (const struct inotify_event *)buff_ptr;

    if (state->event->mask & IN_CREATE) {
      state->msg = "File Created.\n";
    }

    if (state->event->mask & IN_DELETE) {
      state->msg = "File Deleted.\n";
    }

    if (state->event->mask & IN_ACCESS) {
      state->msg = "File Accessed.\n";
    }

    if (state->event->mask & IN_CLOSE) {
      state->msg = "File Closed.\n";
    }

    if (state->event->mask & IN_MODIFY) {
      state->msg = "File Modifed.\n";
    }

    if (state->event->mask & IN_MOVE) {
      state->msg = "File Moved.\n";
    }

    state->handle =
        notify_notification_new(path, state->msg, "dialog-information");

    if (state->handle == NULL) {
      fprintf(stderr, "[WARNING]: Notification handle was null\n");
      continue;
    }

    notify_notification_set_urgency(state->handle, NOTIFY_URGENCY_CRITICAL);
    notify_notification_show(state->handle, NULL);
  }

  return 0;
}
