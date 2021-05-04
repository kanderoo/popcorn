#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <ncurses.h>
#include "config.h"
#include "popcorn.h"
#include "display.h"

#define MEDIA_DIR_SIZE 200
char media_dir[MEDIA_DIR_SIZE];

#define MEDIA_ARR_SIZE 50
struct media media_arr[MEDIA_ARR_SIZE];

int main(int argc, char* argv[]) {
	int title_count;
	store_config();
	title_count = set_media_arr(media_dir, 5, 0); // default recur_depth is 5

	init_display(media_arr, title_count);

	return 0;
}

int set_media_dir(char *dir) {
	strncpy(media_dir, dir, MEDIA_DIR_SIZE);
	return 0;
}

/* returns the length of the media_arr */
int set_media_arr(char* directory, int recur_depth, int array_offset) {
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
				strncpy(title.title, entry->d_name, 100);
				sprintf(title.info, "This is a placeholder description for %s", title.title);
				sprintf(title.path, directory);

				printf("TITLE #%d\n", i);
				printf("\tTitle: %s\n", title.title);
				printf("\tInfo: %s\n", title.info);
				printf("\tPath: %s\n", title.path);

				media_arr[i] = title;

				i++;
			} else if (entry->d_type == DT_DIR && recur_depth > 0) {
				char new_dir[200];
				printf("Recurring into \"%s\"...\n", entry->d_name);

				// construct new directory path to search
				strcat(new_dir, directory);
				strcat(new_dir, "/");
				strcat(new_dir, entry->d_name);

				i += set_media_arr(new_dir, --recur_depth, i);
			}
		}
	} else {
		fprintf(stderr, "Error opening media directory \"%s\"\n", directory);
		return 0;
	}

	return i;
}
