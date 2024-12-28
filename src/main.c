#include "config.h"
#include "event.h"
#include "notify.h"
#include "util.h"
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/syslog.h>
#include <unistd.h>

static volatile sig_atomic_t running = 1;

void handle_signal(int sig) {
  switch (sig) {
  case SIGTERM:
  case SIGINT:
  case SIGKILL:
  case SIGABRT:
    syslog(LOG_INFO, "Exit signal [%d] received", sig);
    running = 0;
    break;
  case SIGHUP:
    syslog(LOG_INFO, "Configuration reload signal [%d] received", sig);
    running = 0;
    break;
  default:
    syslog(LOG_WARNING, "Recieved unexpected signal [%d]", sig);
    break;
  }
}

int main(int argc, char **argv) {
  openlog("file-warden", LOG_PID | LOG_CONS, LOG_DAEMON);
  Config *cfg = init_config();

  struct sigaction sa;
  sa.sa_handler = handle_signal;
  sigemptyset(&sa.sa_mask);

  int sig_status = init_signals(&sa);
  if (sig_status != 0) {
    syslog(LOG_ERR, "Failed to initialize signals");
    free_config(cfg);
    exit(sig_status);
  }

  EventState *state = start_event_listener(cfg);
  if (state == NULL) {
    syslog(LOG_ERR, "Failed to start file event listener");
    free_config(cfg);
    exit(EXT_START_LISTENER);
  }

  int notif_status = init_notif();
  if (notif_status != 0) {
    syslog(LOG_ERR, "Failed to initalize notifications");
    stop_event_listener(state);
    free_config(cfg);
    uninit_notif();
    exit(notif_status);
  }

  // @TODO: rm config debugging once events are working as expected
  debug_config(cfg);
  while (running) {
    state->poll_n = poll(state->fds, state->nfds, POLL_INTERVAL_MS);
    if (state->poll_n == -1) {

      if (errno == EINTR) {
        continue;
      }

      syslog(LOG_ERR, "Failed to poll file descriptor for inotify events");
      running = 0;
    }

    if (state->poll_n > 0 && state->fds[0].revents & POLLIN) {
      int event_status = handle_events(state);

      if (event_status != 0) {
        syslog(LOG_ERR, "Failed to handle file events");
        running = 0;
      }
    }
  }

  syslog(LOG_INFO, "Cleaning up...");
  stop_event_listener(state);
  free_config(cfg);
  uninit_notif();
  exit(EXIT_SUCCESS);
}
