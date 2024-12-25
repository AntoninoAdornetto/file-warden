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
 * [path] the path to the dir or file that [wd] is set too.
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
 * [poll_n] @TODO: create a note that describes [poll_n] responsibility
 * [wd_map] look up table that links inotify watch descriptors to paths. It
 * provides a simple way to determine which path triggered an event.
 * [wd_entry_count] number of active elements contained in [wd_map].
 */
typedef struct {
  int fd;
  int *wd;
  nfds_t nfds;
  struct pollfd fds[1];
  int poll_n;
  WdEntry wd_map[MAX_WATCH_DESCRIPTORS];
  int wd_entry_count;
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
 * Uses input [state] and [wd] to retrieve the path name that is linked to
 * inotify watch descriptor [wd]. If no mapping is found, NULL is returned.
 * Otherwise a pointer to the path is returned from field [wd_map] contained in
 * [state].
 */
char *get_wd_path_mapping(EventState *state, int wd);

#endif
