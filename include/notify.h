#ifndef NOTIFY_H
#define NOTIFY_H

#include <libnotify/notification.h>

/* Exit/Err codes */
#define EXT_INIT_NOTIF 10
#define EXT_CREAT_NOTIF 11
#define EXT_DISPLAY_NOTIF 12

/* Time, in MS, before notification is removed from screen */
#define NOTIF_TIMEOUT_MS 3000

/*
 * Wrapper function to init libnotify, invokes [notify_init]. Returns 0 if
 * successful, otherwise it will return [EXT_INIT_NOTIF] which is then used as
 * an exit code.
 */
int init_notif(void);

/*
 * Wrapper function to uninitialize libnotify, invokes [notify_uninit].
 */
void uninit_notif(void);

/*
 * Creates a new [NotifyNotification] struct using input [title] and [body]
 * as the text that will displayed in the notification.
 */
int display_notification(const char *title, const char *body);

#endif
