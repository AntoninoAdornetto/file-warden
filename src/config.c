#include "config.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syslog.h>

/* Configuration options (tbd if more will be added) */
static const char *OPT_PATHS = "paths";
static const char *OPT_EVENTS = "events";

/* Permitted file events for [OPT_EVENTS] option setting */
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
    syslog(LOG_ERR, "Failed to allocate memory for config struct\n");
    exit(EXT_CFG_ALLOC);
  }

  cfg->paths_size = 0;
  cfg->paths = (char **)malloc(sizeof(char *));
  if (cfg->paths == NULL) {
    syslog(LOG_ERR, "Failed to allocate memory for watch paths\n");
    free_config(cfg);
    exit(EXT_CFG_ALLOC);
  }

  char *program_settings = get_config_settings(cfg);
  if (program_settings == NULL) {
    syslog(LOG_ERR, "Failed to fetch program option settings\n");
    free_config(cfg);
    exit(EXT_NULL_CFG);
  }

  int result = process_settings(cfg, program_settings);
  if (result != 0) {
    syslog(LOG_ERR, "Failed to process program option settings\n");
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

char *get_config_settings(Config *cfg) {
  int in_home = file_exists(CFG_HOME_PATH);
  int in_etc = file_exists(CFG_ETC_PATH);

  // we should use the home configuration first since it's user specific
  // and defer to system configuration if home file is not present.
  if (in_home || in_etc) {
    cfg->config_location = in_home ? CFG_LOC_HOME : CFG_LOC_ETC;

    char *settings = read_file(in_home ? CFG_HOME_PATH : CFG_ETC_PATH);
    if (settings != NULL) {
      syslog(LOG_INFO, "Using config file at [%s]\n",
             in_home ? CFG_HOME_PATH : CFG_ETC_PATH);

      return settings;
    }

    syslog(LOG_WARNING, "Failed to read setting from config file. Deferring to "
                        "default settings\n");
  }

  syslog(LOG_WARNING,
         "Config files may not exist.\nPlease create or copy the example "
         "config.\nDeferring to sensible default settings");

  // Sensible default if the config files are not present.
  cfg->config_location = CFG_LOC_DEFAULT;
  return "paths=~/.ssh\nevents=accessed,modified,moved,closed\n";
}

int process_settings(Config *cfg, const char *settings) {
  if (cfg == NULL) {
    syslog(LOG_ERR, "Expected config struct not to be null\n");
    return EXT_NULL_CFG;
  }

  if (settings == NULL) {
    syslog(LOG_ERR, "Expected program settings not to be empty\n");
    return EXT_NULL_CFG;
  }

  char buf[MAX_OPT_LINE_LEN];
  strcpy(buf, settings);

  char *line = strtok(buf, "\n");
  while (line != NULL) {
    if (process_line_option(cfg, line) != 0) {
      return EXT_PROC_OPTS;
    }
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
    syslog(LOG_ERR, "Invalid option format. Expected \"option=val1,val2\"\n");
    return EXT_OPT_FORMAT;
  }

  strncpy(key, line, equal_sign - line);
  key[equal_sign - line] = '\0';
  u8 opt_flag = validate_option(key);
  if (opt_flag == FLAG_INVAL_OPT) {
    syslog(LOG_WARNING, "Invalid option setting [%s]\n", key);
  }

  strcpy(values, equal_sign + 1);
  token = strtok_r(values, ",", &remaining);
  while (token != NULL) {
    if (set_option(cfg, opt_flag, token) != 0) {
      return EXT_SET_OPT;
    }
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

  syslog(LOG_WARNING, "Invalid option setting with value [%s]\n", setting_key);
  return 0;
}

int set_option(Config *cfg, u8 option_flag, char *value) {
  if (option_flag & FLAG_PATHS_OPT) {
    return set_paths_option(cfg, value);
  }

  if (option_flag & FLAG_EVENTS_OPT) {
    return set_events_option(cfg, value);
  }

  syslog(LOG_WARNING, "skipping value [%s]. Unknown option setting\n", value);
  return 0;
}

int set_paths_option(Config *cfg, char *value) {
  char *expanded_path = expand_path(value);
  if (expanded_path == NULL) {
    syslog(LOG_ERR, "Failed to expand path [%s]\n", value);
    return EXT_SET_PATH_OPT;
  }

  cfg->paths[cfg->paths_size] =
      malloc(sizeof(char) * strlen(expanded_path) + 1);
  if (cfg->paths[cfg->paths_size] == NULL) {
    syslog(LOG_ERR, "Failed to allocate memory for path element at index [%d]",
           cfg->paths_size);
    return EXT_CFG_ALLOC;
  }

  strcpy(cfg->paths[cfg->paths_size], expanded_path);
  free(expanded_path);
  cfg->paths_size++;
  return 0;
}

int set_events_option(Config *cfg, char *value) {
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

  return 0;
}

void debug_config(Config *cfg) {
  if (cfg == NULL) {
    syslog(LOG_DEBUG, "Configuration struct is null\n");
  }

  syslog(LOG_DEBUG, "Paths option settings\n");
  syslog(LOG_DEBUG, "Number of paths set for monitoring [%d]\n",
         cfg->paths_size);

  for (int i = 0; i < cfg->paths_size; i++) {
    syslog(LOG_DEBUG, "Path @ index [%d] contains value [%s]\n", i,
           cfg->paths[i]);
  }
  syslog(LOG_DEBUG, "\n");

  syslog(LOG_DEBUG, "File event option settings\n");
  if (cfg->events_bmask == 0) {
    syslog(LOG_DEBUG, "File event flags are not enabled\n");
    return;
  } else {
    syslog(LOG_DEBUG, "File Event Bit Flags [0x%08X]\n", cfg->events_bmask);
  }

  if (cfg->events_bmask & FLAG_ACCESS) {
    syslog(LOG_DEBUG, "File access event flag enabled\n");
  }

  if (cfg->events_bmask & FLAG_MODIFY) {
    syslog(LOG_DEBUG, "File modify event flag enabled\n");
  }

  if (cfg->events_bmask & FLAG_MOVE) {
    syslog(LOG_DEBUG, "File move event flag enabled\n");
  }

  if (cfg->events_bmask & FLAG_CLOSE) {
    syslog(LOG_DEBUG, "File close event flag enabled\n");
  }
}
