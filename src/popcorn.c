#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <ncurses.h>
#include "config.h"
#include "popcorn.h"
#include "display.h"

int main(int argc, char* argv[]) {
	struct configuration config = default_configuration();

	int title_count;
	store_config(&config);

	if (argc > 1) {
		title_count = read_media_arr(config.media_arr, config.database_path);
	} else {
		title_count = set_media_arr(config.media_arr, config.media_dir, 5, 0); // default recur_depth is 5
	}

	store_media_arr(config.media_arr, config.database_path, title_count);

	init_display(config.media_arr, title_count);

	return 0;
}

/* returns the length of the media_arr */
int set_media_arr(struct media *media_arr, char* directory, int recur_depth, int array_offset) {
	// TODO: put things in the database, but for now just store them in a media array in popcorn.c
	DIR *dir_ptr;
	struct dirent *entry;
	dir_ptr = opendir(directory);
	int i = array_offset;

	if (dir_ptr != NULL) {
		for (entry = readdir(dir_ptr); entry != NULL; entry = readdir(dir_ptr)) {
			if (!strcmp(entry->d_name, "..") || !strcmp(entry->d_name, ".")) continue;

			if (entry->d_type == DT_REG) {
				struct media title;

				// construct a media object
				strncpy(title.title, entry->d_name, 256);
				sprintf(title.info, "This is a placeholder description for %s", title.title);
				sprintf(title.path, directory);

				printf("TITLE #%d\n", i);
				printf("\tTitle: %s\n", title.title);
				printf("\tInfo: %s\n", title.info);
				printf("\tPath: %s\n", title.path);

				media_arr[i] = title;

				i++;
			} else if (entry->d_type == DT_DIR && recur_depth > 0) {
				char new_dir[200] = "\0";
				printf("Recurring into \"%s\"...\n", entry->d_name);

				// construct new directory path to search
				strcat(new_dir, directory);
				strcat(new_dir, "/");
				strcat(new_dir, entry->d_name);

				i = set_media_arr(media_arr, new_dir, --recur_depth, i);
			}
		}
	} else {
		fprintf(stderr, "Error opening media directory \"%s\"\n", directory);
		exit(200);
		return 0;
	}

	return i;
}

int store_media_arr(struct media *media_arr, char *store_file, int size) {
	FILE *fp;
	fp = fopen(store_file, "w+");

	if (fp) {
		for (int i = 0; i < size; i++) {
			fprintf(fp, "%s\n", media_arr[i].title);
			fprintf(fp, "%s\n", media_arr[i].info);
			fprintf(fp, "%s\n", media_arr[i].path);
			fprintf(fp, "\n");
		}
	} else {
		fprintf(stderr, "Error saving the media array");
	}

	return 0;
}

/* returns the size of the new media_arr */
int read_media_arr(struct media *media_arr, char *store_file) {
	FILE *fp;
	fp = fopen(store_file, "r");
	const int BUFFER_SIZE = 256;
	char buffer[BUFFER_SIZE];
	int i = 0;

	if (fp) {
		int number = 0;
		struct media title;
		while (fgets(buffer, BUFFER_SIZE, fp)) {
			if (!strncmp(buffer, "\n", BUFFER_SIZE)) {
				media_arr[i] = title;
				struct media title;
				i++;
				number = 0;
			} else {
				buffer[strcspn(buffer, "\n")] = '\0';
				switch (number) {
					case 0:
						strncpy(title.title, buffer, 256);
						break;
					case 1:
						strncpy(title.info, buffer, 512);
						break;
					case 2:
						strncpy(title.path, buffer, 300);
						break;
				}
				number++;
			}
		}
	} else {
		fprintf(stderr, "Error loading the media array");
	}

	return i;
}
