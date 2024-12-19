#include "util.h"
#include <stdlib.h>

int main(int argc, char **argv) {
  WardenConfig *cfg = process_config();
  free_config(cfg);
  exit(EXIT_SUCCESS);
}
