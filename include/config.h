#ifndef CONFIG_H
#define CONFIG_H
#include "util.h"

/* Configuration option bit flags */
#define FLAG_INVAL_OPT 0x00
#define FLAG_PATHS_OPT 0x01
#define FLAG_EVENTS_OPT 0x02

/* File event bit flags */
#define FLAG_ACCESS 0x01
#define FLAG_MODIFY 0x02
#define FLAG_MOVE 0x04
#define FLAG_CLOSE 0x08

/* Configuration setting line and key/value (option) lengths */
#define MAX_OPT_LINE_LEN 512
#define MAX_OPT_LEN 256
#define OPT_SKIP 69

/* Configuration file location bit flags */
#define CFG_LOC_HOME 0x01
#define CFG_LOC_ETC 0x02
#define CFG_LOC_DEFAULT 0x04

/* Exit Codes */
#define EXT_NULL_CFG 1
#define EXT_CFG_ALLOC 2
#define EXT_ERR_CFG 3
#define EXT_OPT_FORMAT 4

/*
 * [paths] houses the file/dir paths that the daemon will monitor.
 * [paths_size] number of elements contained in [paths].
 * [events_bmask] bit mask that indicates what file events the daemon will
 * trigger notifications for.
 * [config_loc] the config file used for current session. Corresponds to CFG_LOC
 * bit flag
 */
typedef struct {
  char **paths;
  int paths_size;
  u8 events_bmask;
  u8 config_loc;
} Config;

/*
 * Used for enabling bits in [events_bmask]
 */
typedef struct {
  const char *name;
  u8 flag;
} FileEventMapping;

/*
 * Allocates memory for [Config struct] & [paths] property.
 * Reads in setting options from config file, will defer to defaults if the
 * config files do not exist. On [error], the function will
 * free any memory used and exit with a specific exit code.
 */
Config *init_config(void);

/*
 * Clean up after ourselves
 */
void free_config(Config *cfg);

/*
 * Reads program settings/configs in from [~/.file-warden.conf] or
 * [/etc/file-warden.conf] and returns the line separated (key/val) option
 * settings pairs. [e.g "option=value1,value2,value3".] First, home dir file is
 * check, since this is the convention for user specific configs. Next, etc dir
 * is checked since this is the convention for system wide configs. Lastly, if
 * the config files are not present, the function will defer to sensible
 * defaults.
 */
char *get_config_settings(Config *cfg);

/*
 * Setting options are a list of line separated strings. Some settings can have
 * multiple values (separated by commas). Eg. "option=val1,val2"
 */
int process_settings(Config *cfg, const char *settings);

/*
 * String manipulation to extract the option setting key and values.
 */
int process_line_option(Config *cfg, char *line);

/*
 * Compares input value [setting_key] against the allowed option setting
 * strings. If a match is found, a bit flag is returned. If the setting does not
 * result in a match, 0 is returned.
 */
u8 validate_option(char *setting_key);

/*
 * Uses the input flag [option_flag] to determine what option setting should
 * be set. Meaning, if the option flag is for [paths] or [events], it will use
 * the [value] input to set the setting in the [Config struct].
 */
void set_option(Config *cfg, u8 option_flag, char *value);

/*
 * Helper for [set_option] function that is specific to the [paths] setting.
 */
void set_paths_option(Config *cfg, char *value);

/*
 * Helper for [set_option] function that is specific to the [events] setting.
 * It compares the value against allowed file events and enables a specific bit
 * in the [events_bmask] u8 property.
 */
void set_events_option(Config *cfg, char *value);

/*
 * print debugging, just in case :)
 */
void debug_config(Config *cfg);

#endif
