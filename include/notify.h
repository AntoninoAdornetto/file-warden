#ifndef NOTIFY_H
#define NOTIFY_H

#include "util.h"
#include <libnotify/notification.h>
#include <sys/inotify.h>

/* Exit/Err codes */
#define EXT_INIT_NOTIF 10
#define EXT_CREAT_NOTIF 11
#define EXT_DISPLAY_NOTIF 12
#define EXT_SHOW_NOTIF 12

/* Time, in MS, before notification is removed from screen */
#define NOTIF_TIMEOUT_MS 3000

#define MAX_NOTIF_MSG_LEN 256

typedef struct {
  u32 event_flag;
  char title[20];
  char summary[40];
} EventMessageMapping;

/*
 * Wrapper function to init libnotify, invokes [notify_init]. Returns 0 if
 * successful, otherwise it will return [EXT_INIT_NOTIF] exit code.
 */
int init_libnotify(void);

/*
 * Wrapper function to uninitialize libnotify, invokes [notify_uninit].
 */
void uninit_libnotify(void);

/*
 * Processes input [event] and sends corresponding notification to the
 * notification daemon. Returns 0 on success. Otherwise a non-zero int is
 * returned
 */
int notify(const struct inotify_event *event);

/*
 * Sends a desktop notification with the specified [title] and [body]. The
 * function utilizes libnotify apis to create and display a desktop
 * notification.
 */
int send_notification(const char *title, const char *body);

#endif
