#ifndef EVENT_H
#define EVENT_H

#include "config.h"
#include <sys/inotify.h>
#include <sys/poll.h>

#define POLL_INTERVAL_MS 500
#define MAX_WATCH_DESCRIPTORS 50

/* Exit codes */
#define EXT_START_LISTENER 9

/*
 * [wd] inotify watch descriptor
 * [path] uses the [Config] struct to look up the path name of the watch
 * descriptor. inotify event object doesn't provide the name of the path after
 * reading a system event.
 */
typedef struct {
  int wd;
  char *path;
} WdEntry;

/*
 * [fd] inotify file descriptor
 * [wd] list of inotify watch descriptors
 * [nfds] Type used for the number of file descriptors.
 * [fds] describes a polling request.
 * [poll_n] return value of [poll] used to monitor multiple file descriptors if
 * they are ready for reading.
 */
typedef struct {
  int fd;
  int *wd;
  int wd_count;
  nfds_t nfds;
  struct pollfd fds[1];
  int poll_n;
} EventState;

/*
 * Allocates memory for [EventState], creates/initializes a new inotify
 * instance, allocates memory for watch descriptors [wd], adds all paths
 * contained in input [cfg] to inotify watch events, and creates the [WdEntry]
 * mapping. On error, a null pointer is returned and any memory used to create
 * [EventState] is freed. If no error, you are responsible for freeing memory
 * after use. [stop_event_listener] will perform clean up duties.
 */
EventState *start_event_listener(Config *cfg);

/*
 * Closes the inotify file descriptor and frees up allocated memory for input
 * [state].
 */
void stop_event_listener(EventState *state);

/*
 * Handles file system events using the inotify API. It continuously reads
 * events from the inotify file descritor, contained within the input [state],
 * processed each event to determine the type of file operation, retrieves the
 * accosicated file path from the events watch descriptor, and logs information
 * about the event.
 *
 */
int handle_events(EventState *state);

#endif
