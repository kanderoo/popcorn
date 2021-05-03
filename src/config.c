#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include "consts.h"

#define CONFIG_KEY_COUNT 1

char configurable_attributes[CONFIG_KEY_COUNT][16] = {
	"media_dir"
};

int parse_config(char* file_name);
int check_valid_key(char *key);
int assign_key(char *value_ptr);
int store_titles(char *media_dir, int recur_depth);

int main() {
	char *home_dir = getenv("HOME");
	char *xdg_config_dir = getenv("XDG_CONFIG_HOME");
	char config_suffix[16] = "/popcorn/config";
	char config_path[256];

	if (xdg_config_dir) { // TODO: check length of XDG_CONFIG_HOME to avoid buffer overflow
		strcpy(config_path, xdg_config_dir);
	} else {
		fprintf(stderr, "XDG_CONFIG_HOME not specified, using ~/.config/\n");
		strcpy(config_path, home_dir);
	}
	strcat(config_path, config_suffix);

	if (access(config_path, F_OK) == 0) {
		// file exists
		if (parse_config(config_path)) {
			exit(125);
		}
	} else {
		fprintf(stderr, "Config file not found.\n");
		exit(127);
	}

	return 0;
}

int parse_config(char* file_name) {
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

			assign_key(value);
			printf("%s: %s\n", key, value);
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

int assign_key(char *value_ptr) {
	int BUFFER_SIZE = 80;
	char value_buffer[BUFFER_SIZE];
	strncpy(value_buffer, value_ptr, BUFFER_SIZE);

	// can't switch on strings in C so if-else ladder... a preprocessor-based config solution be more elegant?
	if (strncmp(value_buffer, "media_dir", BUFFER_SIZE)) {
		strncpy(media_dir, value_buffer, 200);

		store_titles(media_dir, 5);
	} else {
		// unrecognized key, return 1
		return 1;
	}

	return 0;
}

int store_titles(char *media_dir, int recur_depth) {
	// TODO: put things in the database, but for now just store them in a media array in consts.h
	DIR *dir_ptr;
	struct dirent *entry;
	dir_ptr = opendir(media_dir);

	if (dir_ptr != NULL) {
		int i = 0;
		for (entry = readdir(dir_ptr); entry != NULL; entry = readdir(dir_ptr)) {
			if (!strcmp(entry->d_name, "..") || !strcmp(entry->d_name, ".")) continue;

			if (entry->d_type == DT_REG) {
				struct media title;

				// construct a media object
				strncpy(title.title, entry->d_name, 100);
				sprintf(title.info, "This is a placeholder description for %s", title.title);
				sprintf(title.path, media_dir);

				// display information on the file
				printf("Title: %s\n", title.title);
				printf("Description: %s\n", title.info);
				printf("Relative Path: %s\n\n", title.path);

				media_arr[i] = title;

				i++;
			} else if (entry->d_type == DT_DIR && recur_depth > 0) {
				char new_dir[200];
				printf("Recurring into \"%s\"...\n", entry->d_name);
				strcat(new_dir, media_dir);
				strcat(new_dir, "/");
				strcat(new_dir, entry->d_name);
				store_titles(new_dir, --recur_depth);
			}
		}
	} else {
		fprintf(stderr, "Error opening media directory \"%s\"", media_dir);
		return 1;
	}

	return 0;
}
