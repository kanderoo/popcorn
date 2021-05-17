#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <regex.h>
#include <ctype.h>
#include "popcorn.h"
#include "ui.h"
#include "edit.h"
#include "omdb.h"
#include "stack.h"

int begin_ui(struct media *full_media_arr, char *database_path, int title_count, char* api_key, char* video_player) {
	init_ncurses();
	display_top_bar("Popcorn Movie Manager");
	
	struct media *filtered_media_arr[title_count];
	for (int i = 0; i < title_count; i++) {
		filtered_media_arr[i] = &full_media_arr[i];
	}

	qsort(filtered_media_arr, title_count, sizeof(struct media *), compare_titles);

	begin_stack_layout(full_media_arr, filtered_media_arr, database_path, title_count, api_key, video_player);

	return 0;
}

int init_ncurses() {
	initscr();
	noecho();
	cbreak();
	curs_set(0);
	init_colors();
	keypad(stdscr, TRUE);

	return 0;
}

void init_colors() {
	if (has_colors() == false) {
		endwin();
		printf("Your teminal does not support color.");
		exit(1);
	}
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLUE);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
	init_pair(3, COLOR_BLACK, COLOR_WHITE);
	init_pair(4, COLOR_WHITE, COLOR_CYAN);
}

int request(char* message, char* string, int size) {
	echo();
	nocbreak();
	curs_set(1);

	mvprintw(LINES - 1, 0, message);
	getnstr(string, size);

	noecho();
	cbreak();
	curs_set(0);

	return 0;
}

int confirm(char* message) {
	int response;

	mvprintw(LINES - 1, 0, message);
	switch (getch()) {
		case 'y':
		case 'Y':
			response = 1;
			break;
		case 'n':
		case 'N':
		default:
			response = 0;
	}
	// clear prompt
	move(LINES - 1, 0);
	clrtoeol();

	return response;
}

int display_bottom_bar(char* text) {
	mvprintw(LINES-2, 0, text);
	mvchgat(LINES-2, 0, -1, A_NORMAL, 3, NULL);

	return 0;
}

int display_top_bar(char* text) {
	int length = strlen(text);
	mvprintw(0, (COLS-length)/2, text);
	mvchgat(0, 0, -1, A_BOLD, 1, NULL);

	return 0;
}

int init_titles(struct media **media_arr, int window_width, struct ui_state state) {
	werase(state.title_win);
	display_titles(state.title_win, window_width, media_arr, state.title_count);
	select_title(media_arr, state);
	return 0;
}

int display_titles(WINDOW *window, int window_width, struct media **media_arr, int size) {
	for (int i = 0; i < size; i++) {
		if (strlen(media_arr[i]->title) < window_width) {
			mvwprintw(window, i, 1, "%s (%d)", media_arr[i]->title, media_arr[i]->year);
			mvwprintw(window, i, COLS-6, "%d:%02d", media_arr[i]->runtime / 60, media_arr[i]->runtime % 60);
		} else {
			mvwprintw(window, i, 1, "%.*s...", window_width-3, media_arr[i]->title);
		}
	}

	return 0;
}

int display_info(WINDOW *info_window, struct media title, int index, int total) {
	werase(info_window);

	// horizontal line
	wattron(info_window, COLOR_PAIR(2));
	mvwhline(info_window, 0, 0, 0, COLS);
	mvwprintw(info_window, 0, 4, title.title);
	mvwprintw(info_window, 0, COLS-8, "(%02d/%02d)", index+1, total);
	wattroff(info_window, COLOR_PAIR(2));

	mvwprintw(info_window, 2, 1, "Summary:\n\n");

	int y = 4;

	y += linebreak_string(title.summary, COLS - 7);
	mvwprintstr_indented(info_window, 4, 4, title.summary);
	y++;

	mvwprintw(info_window, y, 1, "Genre: %s", title.genre);
	y += 2;

	mvwprintw(info_window, y, 1, "Year: %d", title.year);
	y += 2;

	mvwprintw(info_window, y, 1, "Runtime: %d Hours, %d Minutes (%d Minutes)", title.runtime / 60, title.runtime % 60, title.runtime);
	y += 2;

	mvwprintw(info_window, y, 1, "Seen: %s", title.watched ? "True":"False");
	y += 2;

	mvwprintw(info_window, y, 1, "Path: %s", title.path);

	return 0;
}

int select_title(struct media **media_arr, struct ui_state state) {
	// highlight title on top box
	wchgat(state.title_win, -1, A_NORMAL, 0, NULL);
	mvwchgat(state.title_win, state.selected_index, 0, -1, A_BOLD, 1, NULL);

	// print info in info window
	display_info(state.info_win, *media_arr[state.selected_index], state.selected_index, state.title_count);

	wrefresh(state.title_win);
	wrefresh(state.info_win);

	return 0;
}

int mvwprintstr_indented(WINDOW *window, int x, int y, char* string) {
	wmove(window, y, x);

	for (int i = 0; i < strlen(string); i++) {
		if (string[i] == '\n') {
			y++;
			wmove(window, y, x);
		} else {
			waddch(window, string[i]);
		}
	}

	return 0;
}

int compare_titles(const void *a, const void *b) {
	int l = tolower((**(struct media **)a).title[0]); // get first character and set to lowercase
	int r = tolower((**(struct media **)b).title[0]);
	return (l - r);
}

int compare_titles_reverse(const void *a, const void *b) {
	int l = tolower((**(struct media **)a).title[0]);
	int r = tolower((**(struct media **)b).title[0]);
	return (r - l);
}

int compare_years(const void *a, const void *b) {
	int l = tolower((**(struct media **)a).year);
	int r = tolower((**(struct media **)b).year);
	return (l - r);
}

int compare_years_reverse(const void *a, const void *b) {
	int l = tolower((**(struct media **)a).year);
	int r = tolower((**(struct media **)b).year);
	return (r - l);
}

/* returns the number of matches */
int filter_titles(struct media *full_media_arr, struct media **filtered_media_arr, int title_count) {
	const int QUERY_SIZE = 512;
	char query[QUERY_SIZE];
	regex_t regex;
	int matches = 0;

	request("#", query, QUERY_SIZE);

	int value = regcomp(&regex, query, 0);

	if (value == 0) {
		// regex compiled successfully
		for (int i = 0; i < title_count; i++) {
			if (regexec(&regex, full_media_arr[i].title, 0, NULL, 0) == 0) {
				// match
				filtered_media_arr[matches] = &full_media_arr[i];
				matches++;
			}
		}
	} else {
		move(LINES - 1, 0);
		clrtoeol();
		mvprintw(LINES - 1, 0, "Error: Could not compile regular expression.");
	}

	return matches;
}

int ui_save(struct media *full_media_arr, char *database_path, int title_count, int *is_changed) {
	if (confirm("Save current media? [Y/n]")) {
		save_media_arr(full_media_arr, database_path, title_count);
		mvprintw(LINES - 1, 0, "Saved!");
		*is_changed = 0;
	}

	return 0;
}

int ui_read(struct media *full_media_arr, struct media **filtered_media_arr, char* database_path, struct ui_state *state) {
	if (!check_save_file_exists(database_path)) {
		mvprintw(LINES - 1, 0, "Database file does not exist, can't read.");
	} else if (!state->is_changed || (state->is_changed && confirm("Reading the database would override changes made in the current session. Read anyway? [y/N]"))) {
		read_media_arr(full_media_arr, database_path);
		state->selected_index = init_titles(filtered_media_arr, COLS, *state);
		mvprintw(LINES - 1, 0, "Database read.");
		state->is_changed = 0;
	}

	return 0;
}

int ui_edit(struct media **filtered_media_arr, struct ui_state *state) {
	def_prog_mode(); // pause ncurses mode
	endwin();
	*filtered_media_arr[state->selected_index] = edit_title(*filtered_media_arr[state->selected_index]);
	state->is_changed = 1;
	reset_prog_mode(); // resume ncurses mode
	init_titles(filtered_media_arr, COLS, *state);
	refresh();

	return 0;
}

int ui_omdb(struct media **filtered_media_arr, char *api_key, struct ui_state *state) {
	if (*api_key) {
		char input[50];
		request("Enter title or IMDB id: ", input, 50);

		def_prog_mode(); // pause ncurses mode
		endwin();
		state->is_changed = get_movie_json(input, api_key, filtered_media_arr[state->selected_index]);
		reset_prog_mode(); // resume ncurses mode

		init_titles(filtered_media_arr, COLS, *state);
		refresh();
	}

	return 0;
}

int ui_open_title(struct media **filtered_media_arr, char *video_player, struct ui_state state) {
	def_prog_mode(); // pause ncurses mode
	endwin();
	filtered_media_arr[state.selected_index]->watched = 1;
	open_video(filtered_media_arr[state.selected_index]->path, video_player);
	reset_prog_mode(); // resume ncurses mode
	init_titles(filtered_media_arr, COLS, state);
	refresh();

	return 0;
}

int ui_search(struct media **filtered_media_arr, struct search_state *search_state, struct ui_state *ui_state) {
	const int QUERY_SIZE = 512;
	char query[QUERY_SIZE];
	regex_t regex;

	request("/", query, QUERY_SIZE);

	int value = regcomp(&regex, query, 0);

	search_state->num_results = 0;
	if (value == 0) {
		// regex compiled successfully
		for (int i = 0; i < ui_state->title_count; i++) {
			if (regexec(&regex, filtered_media_arr[i]->title, 0, NULL, 0) == 0) {
				// match
				search_state->result_indexes[search_state->num_results] = i;
				(search_state->num_results)++;
			}
		}
	} else {
		move(LINES - 1, 0);
		clrtoeol();
		mvprintw(LINES - 1, 0, "Error: Could not compile regular expression.");
	}

	ui_state->selected_index = search_state->result_indexes[0];
	select_title(filtered_media_arr, *ui_state); // select first result
	search_state->result_selected = 0;

	return 0;
}

int ui_next_result(struct media **filtered_media_arr, struct search_state *search_state, struct ui_state *ui_state) {
	if (search_state->num_results > 0) {
		search_state->result_selected = -1;

		// check that there is a later result
		for (int i = 0; i < search_state->num_results && search_state->result_selected == -1; i++) {
			if (search_state->result_indexes[i] > ui_state->selected_index) {
				search_state->result_selected = search_state->result_indexes[i];
			}
		}

		if (search_state->result_selected == -1) {
			mvprintw(LINES-1, 0, "Search hit bottom, continuing at top...");
			search_state->result_selected = search_state->result_indexes[0];
		}

		ui_state->selected_index = search_state->result_selected;
		select_title(filtered_media_arr, *ui_state);
	} else {
		mvprintw(LINES-1, 0, "Not currently searching");
	}

	return 0;
}

int ui_prev_result(struct media **filtered_media_arr, struct search_state *search_state, struct ui_state *ui_state) {
	if (search_state->num_results > 0) {
		search_state->result_selected = -1;

		// check that there is a prev result
		for (int i = search_state->num_results-1; i >= 0 && search_state->result_selected == -1; i--) {
			if (search_state->result_indexes[i] < ui_state->selected_index) {
				search_state->result_selected = search_state->result_indexes[i];
			}
		}

		if (search_state->result_selected == -1) {
			mvprintw(LINES-1, 0, "Search hit top, continuing at bottom...");
			search_state->result_selected = search_state->result_indexes[search_state->num_results-1];
		}

		ui_state->selected_index = search_state->result_selected;
		select_title(filtered_media_arr, *ui_state);
	} else {
		mvprintw(LINES-1, 0, "Not currently searching");
	}

	return 0;
}

int ui_filter(struct media *full_media_arr, struct media **filtered_media_arr, struct ui_state *state) {
	state->title_count = filter_titles(full_media_arr, filtered_media_arr, state->full_title_count);
	init_titles(filtered_media_arr, COLS, *state);
	state->selected_index = 0;

	return 0;
}

int ui_sort_title_toggle(struct media **filtered_media_arr, struct ui_state *state) {
	switch (state->sort_status) {
		case TITLE_FORWARD:
			ui_sort_title(filtered_media_arr, state, TITLE_REVERSE);
			break;
		case TITLE_REVERSE:
		default:
			ui_sort_title(filtered_media_arr, state, TITLE_FORWARD);
			break;
	}
	return 0;
}

int ui_sort_title(struct media **filtered_media_arr, struct ui_state *state, enum sort_type type) {
	switch (type) {
		case TITLE_REVERSE:
			qsort(filtered_media_arr, state->title_count, sizeof(struct media *), compare_titles_reverse);
			state->sort_status = TITLE_REVERSE;
			break;
		case TITLE_FORWARD:
		default:
			qsort(filtered_media_arr, state->title_count, sizeof(struct media *), compare_titles);
			state->sort_status = TITLE_FORWARD;
			break;
	}

	init_titles(filtered_media_arr, COLS, *state);
	state->selected_index = 0;

	return 0;
}

int ui_sort_year_toggle(struct media **filtered_media_arr, struct ui_state *state) {
	switch (state->sort_status) {
		case YEAR_FORWARD:
			ui_sort_year(filtered_media_arr, state, YEAR_REVERSE);
			break;
		case YEAR_REVERSE:
		default:
			ui_sort_year(filtered_media_arr, state, YEAR_FORWARD);
			break;
	}
	return 0;
}

int ui_sort_year(struct media **filtered_media_arr, struct ui_state *state, enum sort_type type) {
	switch (type) {
		case YEAR_REVERSE:
			qsort(filtered_media_arr, state->title_count, sizeof(struct media *), compare_years_reverse);
			state->sort_status = YEAR_REVERSE;
			break;
		case YEAR_FORWARD:
		default:
			qsort(filtered_media_arr, state->title_count, sizeof(struct media *), compare_years);
			state->sort_status = YEAR_FORWARD;
			break;
	}

	init_titles(filtered_media_arr, COLS, *state);
	state->selected_index = 0;

	return 0;
}

int ui_quit(int is_changed) {
	if ((is_changed && confirm("Unsaved changes have been made, quit anyway? [y/N]")) || (!is_changed && confirm("Quit popcorn? [y/N]"))) {
		endwin();
		return 1;
	} 

	return 0;
}
