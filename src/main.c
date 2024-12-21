#include "config.h"
#include "notify.h"
#include <signal.h>
#include <stdlib.h>

volatile sig_atomic_t kill_switch = 0;

void handle_signal(int signal) { kill_switch = 1; }

int main(int argc, char **argv) {
  Config *cfg = init_config();
  debug_config(cfg);

  LibNotifyState *state = init_libnotify(cfg);
  if (state == NULL) {
    free_config(cfg);
    exit(EXT_LIBNOTI_INIT);
  }

  signal(SIGABRT, handle_signal);
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  while (1) {
    int status = watch_libnotify(state, cfg->paths[0]);
    if (status != 0) {
      exit(status);
    }

    if (kill_switch) {
      kill_switch = 0;
      free_config(cfg);
    }
  }

  /*free_config(cfg);*/
  return 0;
}
