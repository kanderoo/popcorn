#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "config.h"
#include "popcorn.h"


#define CONFIG_KEY_COUNT 2

char configurable_attributes[CONFIG_KEY_COUNT][16] = {
	"media_dir",
	"database_path"
};

struct configuration default_configuration() {
	struct configuration default_config;

	sprintf(default_config.media_dir, "%s/Videos", getenv("HOME"));
	sprintf(default_config.database_path, "%s/.config/popcorn/database", getenv("HOME"));

	return default_config;
}

int store_config(struct configuration *config) {
	// parse and store config
	char config_path[256];
	get_config_path(config_path, 256);

	if (access(config_path, F_OK) == 0) {
		// file exists
		if (parse_config(config_path, config)) {
			// parse_config failed
			return 2;
		}
	} else {
		fprintf(stderr, "Config file not found.\n");
		exit(100);
		return 1;
	}

	return 0;
}

int get_config_path(char *config_path, int buffer_size) {
	char *home_dir = getenv("HOME");
	char *xdg_config_dir = getenv("XDG_CONFIG_HOME");
	char config_suffix[16] = "/popcorn/config";

	if (xdg_config_dir) {
		strncpy(config_path, xdg_config_dir, buffer_size);
	} else {
		fprintf(stderr, "XDG_CONFIG_HOME not specified, using ~/.config/\n");
		strcpy(config_path, home_dir);
	}
	strcat(config_path, config_suffix);

	return 0;
}

int parse_config(char* file_name, struct configuration *config) {
	FILE *f;
	const int BUFFER_SIZE = 80;
	f = fopen(file_name, "r");
	if (f == NULL) {
		fprintf(stderr, "Error trying to read config file.\n");
		exit(125);
	}

	char buffer[BUFFER_SIZE];
	char *key;
	char *value;

	while (fgets(buffer, BUFFER_SIZE, f) != NULL) {
		key = strtok(buffer, "\t\n");
		if (key == NULL) continue; // empty line, ignore it
		if (key[0] == '#') continue; // comment, ignore it

		if (check_valid_key(key)) {
			value = strtok(NULL, "\n"); // take rest of line to be value
			if (!value) {
				fprintf(stderr, "Error: No value provided for key \"%s\"\n", key);
				return 1;
			}

			assign_key(key, value, config);
		} else {
			fprintf(stderr, "Warning: \"%s\" in an unrecognized key\n", key);
		}
	}

	fclose(f);

	return 0;
}

int check_valid_key(char *key) {
	int is_valid = 0;
	for (int i = 0; i < CONFIG_KEY_COUNT && is_valid == 0; i++) {
		if (!strcmp(configurable_attributes[i], key)) {
			is_valid = 1;
		}
	}

	return is_valid;
}

int assign_key(char *key_ptr, char *value_ptr, struct configuration *config) {
	// can't switch on strings in C so if-else ladder... a preprocessor-based config solution be more elegant?
	if (!strcmp(key_ptr, "media_dir")) { // needs negated because strcmp returns funky
		strncpy(config->media_dir, value_ptr, 256);
	} else if (!strcmp(key_ptr, "database_path")) {
		strncpy(config->database_path, value_ptr, 256);
	} else {
		// unrecognized key, return 1
		return 1;
	}

	return 0;
}
