#ifndef NOTIFY_H
#define NOTIFY_H

#include "config.h"
#include "util.h"
#include <libnotify/notification.h>
#include <sys/inotify.h>

#define MAX_WATCH_COUNT 10
#define NOTI_BUF_SIZE 4096

/* Exit Codes */
#define EXT_NOTI_ALLOC 5
#define EXT_LIBNOTI_INIT 6
#define EXT_ERR_ADD_WATCH 7
#define EXT_ERR_READ_INOTIFY 8

typedef struct {
  int event_queue;
  int event_status;
  u32 watch_mask;
  char buf[NOTI_BUF_SIZE];
  int read_len;
  char *msg;
  NotifyNotification *handle;
  const struct inotify_event *event;
} LibNotifyState;

typedef struct {
  LibNotifyState lib_notify_state[MAX_WATCH_COUNT];
} Warden;

typedef struct {
  u8 event_mask;
  u32 libnotify_mask;
} LibNotifyEventMapping;

LibNotifyState *init_libnotify(Config *cfg);

/*
 * @TODO: Add new configuration option setting for which notiifcation lib to use
 * There are a number of different libs that could be use with a variety of APIs
 * that we may need to hook into. Libnotify is a common one that we will support
 * out of the gates. We should allow the user to specify which of the supported
 * notification daemons that they are allowed to use.
 */
Warden *init_warden(Config *cfg);

/*
 *
 */
void free_warden(Warden *warden);

/*
 *
 */
int setup_libnotify(Warden *warden, Config *cfg);

/*
 *
 */
void handle_signal(int signal);

int init_watcher(Config *cfg);

int watch_libnotify(LibNotifyState *state, char *path);

u32 create_libnotify_mask(u32 flags);

void debug_libnotifiy_state(void);

#endif
