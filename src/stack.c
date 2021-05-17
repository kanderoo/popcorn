#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include "popcorn.h"
#include "ui.h"
#include "stack.h"
#include "edit.h"
#include "omdb.h"
#include "command.h"

int begin_stack_layout(struct media *full_media_arr, struct media **filtered_media_arr, char *database_path, int title_count, char *api_key, char *video_player) {
	// state setup
	struct ui_state state;
	state.full_title_count = title_count;
	state.title_count = title_count;
	state.is_changed = 0;
	state.selected_index = 0;
	state.sort_status = TITLE_FORWARD;

	// init windows
	const int TOP_PANEL_HEIGHT = (LINES - 2)/2;
	WINDOW *top_panel = newwin(TOP_PANEL_HEIGHT, COLS, 1, 0);
	WINDOW *info_panel = newwin(LINES-(TOP_PANEL_HEIGHT+3), COLS, TOP_PANEL_HEIGHT+1, 0);

	state.title_win = top_panel;
	state.info_win = info_panel;
	refresh();

	// bottom bar
	if (*api_key) {
		display_bottom_bar("q: quit, e: edit, s: save, r: load, o: get data from OMDB");
	} else {
		display_bottom_bar("q: quit, e: edit, s: save, r: load");
	}

	// top box setup
	state.selected_index = init_titles(filtered_media_arr, COLS, state);

	// search setup
	struct search_state search_state;
	int result_indexes[title_count];
	search_state.num_results = 0;
	search_state.result_indexes = result_indexes;
	search_state.result_selected = 0;

	// get user input
	int ch;
	while (1) {
		ch = getch();
		// clear prompt on keypress
		move(LINES - 1, 0);
		clrtoeol();
		switch (ch) {
			case 'j':
			case KEY_DOWN: // move down
				if (state.selected_index < state.title_count - 1) state.selected_index++;
				select_title(filtered_media_arr, state);
				break;
			case 'k':
			case KEY_UP: // move up
				if (state.selected_index > 0) state.selected_index--;
				select_title(filtered_media_arr, state);
				break;
			case 'g':
			case KEY_HOME: // go to top
				state.selected_index = 0;
				select_title(filtered_media_arr, state);
				break;
			case 'G':
			case KEY_END: // go to bottom
				state.selected_index = state.title_count - 1;
				select_title(filtered_media_arr, state);
				break;
			case 's': // save media array
				ui_save(full_media_arr, database_path, title_count, &state.is_changed);
				break;
			case 'r':
				ui_read(full_media_arr, filtered_media_arr, database_path, &state);
				break;
			case 'e': // edit title
				ui_edit(filtered_media_arr, &state);
				break;
			case 'o': // get omdb data
				ui_omdb(filtered_media_arr, api_key, &state);
				break;
			case KEY_ENTER: // open movie
			case '\n':
				ui_open_title(filtered_media_arr, video_player, state);
				break;
			case '/': // search
				ui_search(filtered_media_arr, &search_state, &state);
				break;
			case 'n': // goto next search result
				ui_next_result(filtered_media_arr, &search_state, &state);
				break;
			case 'N': // goto previous search result
				ui_prev_result(filtered_media_arr, &search_state, &state);
				break;
			case 'f': // filter
				ui_filter(full_media_arr, filtered_media_arr, &state);
				break;
			case 't': // sort title
				ui_sort_title_toggle(filtered_media_arr, &state);
				break;
			case 'y': // sort year
				ui_sort_year_toggle(filtered_media_arr, &state);
				break;
			case ':': // command
				if (command(full_media_arr, filtered_media_arr, database_path, api_key, video_player, &state) == 1) {
					return 0;
				}
				break;
			case 'q': // quit
				if (ui_quit(state.is_changed) == 1) {
					return 0;
				}
				break;
		}
	}

	return 0;
}
