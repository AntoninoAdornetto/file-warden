#include "config.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* Configuration options (tbd if more will be added) */
static const char *OPT_PATHS = "paths";
static const char *OPT_EVENTS = "events";
static const char *EVENT_ACCESS = "accessed";
static const char *EVENT_MODIFY = "modified";
static const char *EVENT_MOVE = "moved";
static const char *EVENT_CLOSE = "closed";

/*
 * Configuration files can be stored in either location.
 * Home dir will be user specific whereas /etc is system wide.
 * Sensible default settings are used if the conf files don't exist.
 */
static const char *CFG_ETC_PATH = "/etc/file-warden.conf";
static const char *CFG_HOME_PATH = "~/.file-warden.conf";

Config *init_config(void) {
  Config *cfg = (Config *)malloc(sizeof(Config));
  if (cfg == NULL) {
    fprintf(stderr, "[ERROR]: Failed to allocate memory for config struct\n");
    exit(EXT_CFG_ALLOC);
  }

  cfg->paths_size = 0;
  cfg->paths = (char **)malloc(sizeof(char *));
  if (cfg->paths == NULL) {
    fprintf(stderr, "[ERROR]: Failed to allocate memory for paths property\n");
    free_config(cfg);
    exit(EXT_CFG_ALLOC);
  }

  char *program_settings = read_config();
  if (program_settings == NULL) {
    fprintf(stderr, "[ERROR]: Failed to set default settings as fallback\n");
    free_config(cfg);
    exit(EXT_NULL_CFG);
  }

  int result = process_settings(cfg, program_settings);
  if (result != 0) {
    fprintf(stderr, "[ERROR]: Failed to process program settings\n");
    free_config(cfg);
    exit(result);
  }

  return cfg;
}

void free_config(Config *cfg) {
  if (cfg == NULL) {
    return;
  }

  for (int i = 0; i < cfg->paths_size; i++) {
    free(cfg->paths[i]);
  }

  free(cfg);
}

char *read_config(void) {
  int in_home = file_exists(CFG_HOME_PATH);
  int in_etc = file_exists(CFG_ETC_PATH);

  if (in_home) {
    // @TODO:(#3) read user configuration file in from ~/.file-warden.conf
    return "";
  }

  if (in_etc) {
    // @TODO:(#2) read system configuration file in from /etc/file-warden.conf
    return "";
  }

  fprintf(
      stdout,
      "[WARNING]: Configuration files do not exist. \n[INFO]: Please create "
      "or copy the example config to [/etc/file-warden.conf] or "
      "[~/.file-warden.conf].\n[INFO]: Deferring to default settings\n\n");

  // Defaults we will resort to if the config files are not present.
  return "paths=~/.ssh/\nevents=accessed,modified,moved,closed\n";
}

int process_settings(Config *cfg, const char *settings) {
  if (cfg == NULL) {
    fprintf(stderr, "[ERROR]: Expected config struct not to be null\n");
    return EXT_NULL_CFG;
  }

  if (settings == NULL) {
    fprintf(stderr, "[ERROR]: Expected program settings not to be empty\n");
    return EXT_NULL_CFG;
  }

  char buf[MAX_OPT_LINE_LEN];
  strcpy(buf, settings);

  char *line = strtok(buf, "\n");
  while (line != NULL) {
    process_line_option(cfg, line);
    line = strtok(NULL, "\n");
  }

  return 0;
}

int process_line_option(Config *cfg, char *line) {
  char key[MAX_OPT_LEN];
  char values[MAX_OPT_LEN];
  char *token;
  char *remaining;

  char *equal_sign = strchr(line, '=');
  if (equal_sign == NULL) {
    fprintf(stderr,
            "[ERROR]: Invalid option format. Expected \"option=val1,val2\"\n");
    return EXT_OPT_FORMAT;
  }

  strncpy(key, line, equal_sign - line);
  key[equal_sign - line] = '\0';
  u8 opt_flag = validate_option(key);
  if (opt_flag == FLAG_INVAL_OPT) {
    fprintf(stdout, "[WARNING]: Invalid option setting [%s]\n", key);
  }

  strcpy(values, equal_sign + 1);
  token = strtok_r(values, ",", &remaining);
  while (token != NULL) {
    set_option(cfg, opt_flag, token);
    token = strtok_r(NULL, ",", &remaining);
  }

  return 0;
}

u8 validate_option(char *setting_key) {
  if (strcmp(setting_key, OPT_PATHS) == 0) {
    return FLAG_PATHS_OPT;
  }

  if (strcmp(setting_key, OPT_EVENTS) == 0) {
    return FLAG_EVENTS_OPT;
  }

  return 0;
}

void set_option(Config *cfg, u8 option_flag, char *value) {
  if (option_flag & FLAG_INVAL_OPT) {
    return;
  }

  if (option_flag & FLAG_PATHS_OPT) {
    set_paths_option(cfg, value);
    return;
  }

  if (option_flag & FLAG_EVENTS_OPT) {
    set_events_option(cfg, value);
    return;
  }
}

void set_paths_option(Config *cfg, char *value) {
  cfg->paths[cfg->paths_size] = malloc(sizeof(char) * strlen(value) + 1);
  if (cfg->paths[cfg->paths_size] == NULL) {
    fprintf(
        stderr,
        "[ERROR]: Failed to allocate memory for paths value at index [%d]\n",
        cfg->paths_size);
    free_config(cfg);
    return;
  }

  strcpy(cfg->paths[cfg->paths_size], value);
  cfg->paths_size++;
}

void set_events_option(Config *cfg, char *value) {
  const FileEventMapping events[] = {{EVENT_ACCESS, FLAG_ACCESS},
                                     {EVENT_MODIFY, FLAG_MODIFY},
                                     {EVENT_MOVE, FLAG_MOVE},
                                     {EVENT_CLOSE, FLAG_CLOSE},
                                     {NULL, 0}};

  for (int i = 0; events[i].name != NULL; i++) {
    if (strcmp(value, events[i].name) == 0) {
      cfg->events_bmask |= events[i].flag;
      break;
    }
  }
}

void debug_config(Config *cfg) {
  if (cfg == NULL) {
    fprintf(stderr, "[DEBUG]: Configuration struct is null\n");
  }

  fprintf(stdout, "[DEBUG]: Paths option settings\n");
  fprintf(stdout, "[DEBUG]: Number of paths set for monitoring [%d]\n",
          cfg->paths_size);

  for (int i = 0; i < cfg->paths_size; i++) {
    fprintf(stdout, "[DEBUG]: Path @ index [%d] contains value [%s]\n", i,
            cfg->paths[i]);
  }
  fprintf(stdout, "\n");

  fprintf(stdout, "[DEBUG]: File event option settings\n");
  if (cfg->events_bmask == 0) {
    fprintf(stderr, "[WARNING]: File event flags are not enabled\n");
    return;
  } else {
    fprintf(stdout, "[DEBUG]: File Event Bit Flags [0x%08X]\n",
            cfg->events_bmask);
  }

  if (cfg->events_bmask & FLAG_ACCESS) {
    fprintf(stdout, "[DEBUG]: File access event flag enabled\n");
  }

  if (cfg->events_bmask & FLAG_MODIFY) {
    fprintf(stdout, "[DEBUG]: File modify event flag enabled\n");
  }

  if (cfg->events_bmask & FLAG_MOVE) {
    fprintf(stdout, "[DEBUG]: File move event flag enabled\n");
  }

  if (cfg->events_bmask & FLAG_CLOSE) {
    fprintf(stdout, "[DEBUG]: File close event flag enabled\n");
  }
}
