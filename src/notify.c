#include "notify.h"
#include "glib-object.h"
#include <libnotify/notification.h>
#include <libnotify/notify.h>
#include <stdbool.h>
#include <sys/syslog.h>

int init_notif(void) {
  if (!(notify_init("file-warden"))) {
    syslog(LOG_ERR, "Failed to initialize libnotify\n");
    return EXT_INIT_NOTIF;
  }

  return 0;
}

void uninit_notif(void) { notify_uninit(); }

int display_notification(const char *title, const char *body) {
  NotifyNotification *notif = notify_notification_new(title, body, NULL);
  if (notif == NULL) {
    syslog(LOG_ERR, "Failed to create a new libnotify notification instance\n");
    return EXT_CREAT_NOTIF;
  }

  notify_notification_set_timeout(notif, NOTIF_TIMEOUT_MS);
  notify_notification_set_urgency(notif, NOTIFY_URGENCY_NORMAL);

  bool status = notify_notification_show(notif, NULL);
  if (!status) {
    syslog(LOG_ERR, "Failed to display libnotify notification\n");
    g_object_unref(notif);
    return EXT_DISPLAY_NOTIF;
  }

  g_object_unref(notif);
  return 0;
}
