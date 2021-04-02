#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "config.h"

char configurable_attributes[1][16] = {
	"media_dir"
}

int main() {
	char *home_dir = getenv("HOME");
	char *xdg_config_dir = getenv("XDG_CONFIG_HOME");
	char config_suffix[16] = "/popcorn/config";
	char config_path[100];

	if (xdg_config_dir) { // TODO: check length of XDG_CONFIG_HOME to avoid buffer overflow
		strcpy(config_path, xdg_config_dir);
	} else {
		fprintf(stderr, "XDG_CONFIG_HOME not specified, using ~/.config/\n");
		strcpy(config_path, home_dir);
	}
	strcat(config_path, config_suffix);

	if (access(config_path, F_OK) == 0) {
		parse_config(config_path);
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
	char *comment_start = NULL;

	while (fgets(buffer, BUFFER_SIZE, f) != NULL) {
		if ((comment_start = strchr('#')) == NULL && commentStart[commentStart - sizeof(char)] == '\') {
			comment_start = NULL;
		}
		printf("%s", buffer);
	}

	fclose(f);

	return 0;
}
