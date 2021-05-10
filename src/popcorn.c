#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <ncurses.h>

#include "popcorn.h"
#include "config.h"
#include "ui.h"
#include "edit.h"

int main(int argc, char* argv[]) {
	struct configuration config = default_configuration();
	struct media media_arr[50]; // TODO: deprecate this when moving to sqlite database

	int title_count;
	store_config(&config);

	if (check_save_file_exists(config.database_path)) {
		title_count = read_media_arr(media_arr, config.database_path);
	} else {
		printf("Database file empty. Scan through \"%s\" to populate it? [Y/n]", config.media_dir);
		int response = getchar();
		while ((getchar()) != '\n'); // consume newline
		switch (response) {
			case 'y':
			case 'Y':
				printf("Searching thorough media directory (%s)...\n", config.media_dir);
				title_count = scan_media_dir(media_arr, config.media_dir, 5, 0, 1); // default recur_depth is 5
				save_media_arr(media_arr, config.database_path, title_count);
				break;
			default:
				break;
		}
	}

	// send control to ui.c
	begin_ui(media_arr, config.database_path, title_count);

	return 0;
}

/* returns the number of lines in the new string */
int linebreak_string(char* string, int line_length) {
	int last_space_index = 0;
	int column = 1;
	int lines = 1;

	for (int i = 0; i < strlen(string); i++) {
		if (string[i] == ' ') {
			last_space_index = i;
		}

		if (column == line_length) {
			string[last_space_index] = '\n';
			column = 1;
			lines++;
		} else {
			column++;
		}
	}

	return lines;
}

struct media construct_default_title(char* filename, char* directory) {
	// TODO: read metadata from file
	struct media title;

	strncpy(title.title, filename, 256);
	sprintf(title.summary, "This is a placeholder description for %s", title.title);
	strncpy(title.genre, "None", 32);
	title.year = 0;
	title.watched = 0;
	title.runtime = 0;
	title.type = MOVIE;
	sprintf(title.path, directory);

	return title;
}

/* returns the length of the media_arr */
int scan_media_dir(struct media *media_arr, char* directory, int recur_depth, int array_offset, int request_edits) {
	// TODO: put things in the database, but for now just store them in a media array in popcorn.c
	DIR *dir_ptr;
	struct dirent *entry;
	dir_ptr = opendir(directory);
	int i = array_offset;
	int response;

	if (dir_ptr != NULL) {
		for (entry = readdir(dir_ptr); entry != NULL; entry = readdir(dir_ptr)) {
			if (!strcmp(entry->d_name, "..") || !strcmp(entry->d_name, ".")) continue;

			if (entry->d_type == DT_REG) {
				struct media new_title = construct_default_title(entry->d_name, directory);
				if (request_edits) {
					printf("\n");
					populate_temp_edit_file(new_title, stdout); // arguably using this function here is confusing to read, but it will be changed in the sqlite conversion anyway
					printf("New title found, would you like to edit it's metadata? (a: no to all) [Y/n/a] ");

					response = getchar();
					while ((getchar()) != '\n'); // consume newline
					switch(response) {
						case 'y':
						case 'Y':
							media_arr[i] = edit_title(new_title);
							break;
						case 'a':
							request_edits = 0;
							media_arr[i] = new_title;
							break;
						case 'n':
						case 'N':
						default:
							media_arr[i] = new_title;
							break;
					}
				} else {
					media_arr[i] = new_title;
				}
				i++;
			} else if (entry->d_type == DT_DIR && recur_depth > 0) {
				char new_dir[200] = "\0";
				printf("Recurring into \"%s\"...\n", entry->d_name);

				// construct new directory path to search
				strcat(new_dir, directory);
				strcat(new_dir, "/");
				strcat(new_dir, entry->d_name);

				i = scan_media_dir(media_arr, new_dir, --recur_depth, i, request_edits);
			}
		}
	} else {
		fprintf(stderr, "Error opening media directory \"%s\"\n", directory);
		exit(200);
		return 0;
	}

	closedir(dir_ptr);
	return i;
}

int save_media_arr(struct media *media_arr, char *store_file, int size) {
	FILE *fp;
	fp = fopen(store_file, "w+");

	if (fp) {
		for (int i = 0; i < size; i++) {
			fprintf(fp, "%s\n", media_arr[i].title);
			fprintf(fp, "%s\n", media_arr[i].summary);
			fprintf(fp, "%s\n", media_arr[i].genre);
			fprintf(fp, "%d\n", media_arr[i].year);
			fprintf(fp, "%d\n", media_arr[i].watched);
			fprintf(fp, "%d\n", media_arr[i].runtime);
			fprintf(fp, "%s\n", media_arr[i].path);
			fprintf(fp, "\n");
		}
	} else {
		fprintf(stderr, "Error saving the media array");
	}

	fclose(fp);

	return 0;
}

/* returns the size of the new media_arr */
int read_media_arr(struct media *media_arr, char *store_file) {
	FILE *fp;
	fp = fopen(store_file, "r");
	const int BUFFER_SIZE = 1024 + 1;
	char buffer[BUFFER_SIZE];
	int i = 0;
	char *remainder;

	if (fp) {
		int number = 0;
		struct media title;
		while (fgets(buffer, BUFFER_SIZE, fp)) {
			if (!strncmp(buffer, "\n", BUFFER_SIZE)) {
				media_arr[i] = title;
				i++;
				number = 0;
			} else {
				buffer[strcspn(buffer, "\n")] = '\0';
				switch (number) {
					case 0:
						strncpy(title.title, buffer, 256);
						break;
					case 1:
						strncpy(title.summary, buffer, 1024);
						break;
					case 2:
						strncpy(title.genre, buffer, 32);
						break;
					case 3:
						title.year = strtol(buffer, &remainder, 10);
						break;
					case 4:
						title.watched = strtol(buffer, &remainder, 10);
						break;
					case 5:
						title.runtime = strtol(buffer, &remainder, 10);
						break;
					case 6:
						strncpy(title.path, buffer, 300);
						break;
					default:
						break;
				}
				number++;
			}
		}
	} else {
		fprintf(stderr, "Error loading the media array");
	}

	fclose(fp);

	return i;
}

int check_save_file_exists(char *file_path) {
	return (access(file_path, F_OK) == 0);
}
