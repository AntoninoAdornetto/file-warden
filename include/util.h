#ifndef UTIL_H
#define UTIL_H
#include <stdint.h>

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

/*
 * [paths] houses the file paths and dir paths that the daemon will monitor.
 * [flags] bit flags that indicate what events the daemon will trigger
 * notifications.
 */
typedef struct {
  char **paths;
  u16 flags;
} WardenConfig;

WardenConfig *process_config(void);
void free_config(WardenConfig *cfg);

#endif
