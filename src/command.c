#include <string.h>
#include <ncurses.h>
#include "popcorn.h"
#include "ui.h"

int command(struct media *full_media_arr, struct media **filtered_media_arr, char *database_path, char *api_key, char *video_player, struct ui_state *state) {
	const int BUFFER_SIZE = 80;
	char input[BUFFER_SIZE];

	request(":", input, BUFFER_SIZE);

	char* command = strtok(input, " \n");

	// this makes me sad but i'm not sure of a better way to do it
	if (strncmp(command, "save", BUFFER_SIZE) == 0) {
		save_media_arr(full_media_arr, database_path, state->title_count);
		mvprintw(LINES - 1, 0, "Saved!");
		state->is_changed = 0;
	} else if (strncmp(command, "read", BUFFER_SIZE) == 0) {
		ui_read(full_media_arr, filtered_media_arr, database_path, state);
	} else if (strncmp(command, "edit", BUFFER_SIZE) == 0) {
		ui_edit(filtered_media_arr, state);
	} else if (strncmp(command, "omdb", BUFFER_SIZE) == 0) {
		ui_omdb(filtered_media_arr, api_key, state);
	} else if (strncmp(command, "open", BUFFER_SIZE) == 0) {
		ui_open_title(filtered_media_arr, video_player, *state);
	} else if (strncmp(command, "filter", BUFFER_SIZE) == 0) {
		ui_filter(full_media_arr, filtered_media_arr, state);
	} else if (strncmp(command, "sort", BUFFER_SIZE) == 0) {
		ui_sort_title_toggle(filtered_media_arr, state);
	} else if (strncmp(command, "quit", BUFFER_SIZE) == 0) {
		ui_quit(state->is_changed);
		return 1;
	}

	return 0;
}
