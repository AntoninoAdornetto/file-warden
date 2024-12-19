#include "util.h"
#include <stdlib.h>

/*
 * @TODO:(#1) Handle Config paths array memory allocation/deallocation
 * the paths array is not utilized at the moment. I am determining how I want
 * to design the system and will circle back to it once I am further along in
 * development
 */

WardenConfig *process_config(void) {
  WardenConfig *cfg = (WardenConfig *)malloc(sizeof(WardenConfig));
  return cfg;
}

void free_config(WardenConfig *cfg) {
  if (cfg != NULL) {
    free(cfg);
  }
}
