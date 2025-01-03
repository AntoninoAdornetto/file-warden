#ifndef CONFIG_H
#define CONFIG_H

#include "util.h"

#define FLAG_INVAL_OPT 0x00  // Invalid configuration option
#define FLAG_PATHS_OPT 0x01  // paths we will enable fs event watchers on
#define FLAG_EVENTS_OPT 0x02 // events that we would like notification for

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
#define EXT_PROC_OPTS 5
#define EXT_SET_OPT 6
#define EXT_SET_PATH_OPT 7

/*
 * [paths] houses the file/dir paths that the daemon will monitor.
 * [paths_size] number of elements contained in [paths].
 * [events_mask] bit mask that indicates which file system events we want to
 * listen for. The flags are defined by inotify, e.g., IN_ACCESS, IN_OPEN,
 * IN_CLOSE, etc...
 * [config_location] the config file used for current session. Corresponds to
 * CFG_LOC bit flag.
 */
typedef struct {
  char **paths;
  int paths_size;
  u32 events_mask;
  u32 config_location;
} Config;

/*
 * Maps a File System event string to corresponding bit flag. The flags are
 * defined by inotify. The mapping will help produce a bit mask that is suitable
 * for inotify to invoke [inotify_add_watch].
 */
typedef struct {
  char *name;
  u32 flag;
} FSEventMapping;

/*
 * Allocates memory for [Config] struct and [paths] property.
 * Attempts to read program option settings from a config file ([see
 * get_config_settings] for more details). On error, the program will free any
 * memory used and exit. If no error, you are responsible for freeing memory
 * after use.
 */
Config *init_config(void);

/*
 * Clean up after ourselves
 */
void free_config(Config *cfg);

/*
 * Reads option settings/configs in from [~/.file-warden.conf] or
 * [/etc/file-warden.conf] and returns line delimited option settings for the
 * program to base it's settings from. Each option setting will be in key/val
 * format, separated by equals sign. [e.g., "option=value1,value2,value3"].
 * First, home dir config file is checked. If it doesn't exist, we will check in
 * /etc dir. Otherwise, the program will defer to sesnsible default settings
 * that will monitor the ~/.ssh dir for file events [written, moved].
 */
char *get_config_settings(Config *cfg);

/*
 * Processes the new line delimited option [settings] input.
 * Returns 0 if the function is able to successfully map all the option settings
 * into their correct fields ([paths], [events]). On error, -1 is returned.
 * NOTE: Some settings can have multiple values (separated by commas).
 * Others could be a single key/val pair. [e.g., "option=value,value2" or
 * "option=value"].
 */
int process_settings(Config *cfg, const char *settings);

/*
 * Util function for [process_settings] that accepts a [line] as input
 * and extracts the key (option). Returns 0 if successful, otherwise -1.
 */
int process_line_option(Config *cfg, char *line);

/*
 * Compares input [setting_key] against permitted option settings. If the input
 * matches an option, a non zero bit flag is returned. If no match is found, 0
 * is returned.
 */
u32 validate_option(char *setting_key);

/*
 * Determines which field (in [cfg]) that [value] should be mapped too.
 * [option_flag] is used to pin-point the field that should be used. Returns 0
 * if a property in [cfg] stores [value] in a property. Otherwise -1 is
 * returned.
 */
int set_option(Config *cfg, u32 option_flag, char *value);

/*
 * Util function for [set_option] that is specifc to appending path [value] to
 * [cfg->paths].
 */
int set_paths_option(Config *cfg, char *value);

/*
 * Helper for [set_option] function that is specific to the [events] setting.
 * It compares the value against allowed file events and enables a specific bit
 * in the [events_bmask] u8 property. Shouldn't error and will return 0.
 */
int set_events_option(Config *cfg, char *value);

/*
 * print debugging, just in case :)
 */
void debug_config(Config *cfg);

#endif
