#include "notify.h"
#include "glib-object.h"
#include <libnotify/notification.h>
#include <libnotify/notify.h>
#include <stdbool.h>
#include <string.h>
#include <sys/syslog.h>

int init_libnotify(void) {
  if (!(notify_init("file-warden"))) {
    syslog(LOG_ERR, "Failed to initialize libnotify");
    return EXT_INIT_NOTIF;
  }

  return 0;
}

void uninit_libnotify(void) { notify_uninit(); }

int send_notification(const char *title, const char *body) {
  NotifyNotification *notif = notify_notification_new(title, body, NULL);
  if (notif == NULL) {
    syslog(LOG_ERR, "Failed to create a new libnotify notification instance");
    return EXT_CREAT_NOTIF;
  }

  notify_notification_set_timeout(notif, NOTIF_TIMEOUT_MS);
  notify_notification_set_urgency(notif, NOTIFY_URGENCY_NORMAL);

  bool status = notify_notification_show(notif, NULL);
  if (!status) {
    syslog(LOG_ERR, "Failed to display libnotify notification");
    g_object_unref(notif);
    return EXT_DISPLAY_NOTIF;
  }

  g_object_unref(notif);
  return 0;
}

int notify(const struct inotify_event *event) {
  if (event->len <= 0) {
    return 0;
  }

  size_t remaining = MAX_NOTIF_MSG_LEN - 1;
  char notif_msg[MAX_NOTIF_MSG_LEN] = {0};

  if (strlen(event->name) >= remaining) {
    syslog(LOG_ERR, "Event name exceeds max summary message length [%d]",
           MAX_NOTIF_MSG_LEN);
    return EXT_SHOW_NOTIF;
  }

  if (event->mask & IN_ISDIR) {
    strncat(notif_msg, "Directory (", remaining);
  } else {
    strncat(notif_msg, "File (", remaining);
  }

  remaining -= strlen(notif_msg) - 1;
  strncat(notif_msg, event->name, remaining);

  EventMessageMapping ev[7] = {
      {IN_OPEN, "Open Alert!", ") was opened!"},
      {IN_CLOSE_WRITE, "Close Alert!", ") was written to and closed!"},
      {IN_CLOSE_NOWRITE, "Close Alert!", ") was not written to and closed!"},
      {IN_CREATE, "Create Alert!", ") was created!"},
      {IN_DELETE, "Delete Alert!", ") was deleted!"},
      {IN_MOVE, "Move Alert!", ") was moved!"},
      {0, "", ""},
  };

  for (int i = 0; ev[i].event_flag != 0; i++) {
    if (event->mask & ev[i].event_flag) {
      remaining -= strlen(notif_msg) - 1;
      strncat(notif_msg, ev[i].summary, remaining);

      syslog(LOG_INFO, "%s %s", ev[i].title, ev[i].summary);
      return send_notification(ev[i].title, notif_msg);
    }
  }

  return 0;
}
