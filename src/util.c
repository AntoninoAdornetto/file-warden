#include "util.h"
#include <stdlib.h>

WardenConfig *process_config(void) {
  WardenConfig *cfg = (WardenConfig *)malloc(sizeof(WardenConfig));
  return cfg;
}

void free_config(WardenConfig *cfg) {
  if (cfg != NULL) {
    free(cfg);
  }
}
