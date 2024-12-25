#include "config.h"
#include "event.h"
#include "util.h"
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
    syslog(LOG_INFO, "Exit signal [%d] received\n", sig);
    running = 0;
    break;
  case SIGHUP:
    syslog(LOG_INFO, "Configuration reload signal [%d] received\n", sig);
    running = 0;
    break;
  default:
    syslog(LOG_WARNING, "Recieved unexpected signal [%d]\n", sig);
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
    syslog(LOG_ERR, "Failed to initialize signals\n");
    free_config(cfg);
    exit(sig_status);
  }

  EventState *state = start_event_listener(cfg);
  if (state == NULL) {
    syslog(LOG_ERR, "Failed to start file event listener\n");
    free_config(cfg);
    exit(EXT_START_LISTENER);
  }

  while (running) {
    debug_config(cfg);
    sleep(120);
  }

  syslog(LOG_INFO, "Cleaning up...\n");
  stop_event_listener(state);
  free_config(cfg);
  exit(EXIT_SUCCESS);
}
