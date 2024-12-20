#include "config.h"
#include <stdlib.h>

int main(int argc, char **argv) {
  Config *cfg = init_config();
  debug_config(cfg);
  free_config(cfg);
  exit(EXIT_SUCCESS);
}
