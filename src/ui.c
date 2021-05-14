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

int begin_stack_layout(struct media *full_media_arr, struct media **filtered_media_arr, char *database_path, int title_count, char *api_key, char *video_player) {
	const int full_title_count = title_count;
	int is_changed = 0;
	enum sort_type sort = TITLE_FORWARD;

	// init windows
	const int TOP_PANEL_HEIGHT = (LINES - 2)/2;
	WINDOW *top_panel = newwin(TOP_PANEL_HEIGHT, COLS, 1, 0);
	WINDOW *info_panel = newwin(LINES-(TOP_PANEL_HEIGHT+3), COLS, TOP_PANEL_HEIGHT+1, 0);
	refresh();

	// bottom bar
	if (*api_key) {
		display_bottom_bar("q: quit, e: edit, s: save, r: load, o: get data from OMDB");
	} else {
		display_bottom_bar("q: quit, e: edit, s: save, r: load");
	}

	// top box setup
	int selected_index = init_titles(top_panel, info_panel, COLS, filtered_media_arr, title_count, 0);

	// search setup
	int num_results = 0;
	int result_indexes[title_count];
	int result_selected = 0;

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
				if (selected_index < title_count - 1) selected_index++;
				select_title(top_panel, info_panel, filtered_media_arr, selected_index, title_count);
				break;
			case 'k':
			case KEY_UP: // move up
				if (selected_index > 0) selected_index--;
				select_title(top_panel, info_panel, filtered_media_arr, selected_index, title_count);
				break;
			case 'g':
			case KEY_HOME: // go to top
				selected_index = 0;
				select_title(top_panel, info_panel, filtered_media_arr, selected_index, title_count);
				break;
			case 'G':
			case KEY_END: // go to bottom
				selected_index = title_count - 1;
				select_title(top_panel, info_panel, filtered_media_arr, selected_index, title_count);
				break;
			case 's': // save media array
				if (confirm("Save current media? [Y/n]")) {
					save_media_arr(full_media_arr, database_path, title_count);
					mvprintw(LINES - 1, 0, "Saved!");
					is_changed = 0;
				}
				break;
			case 'r': // read media array
				if (!check_save_file_exists(database_path)) {
					mvprintw(LINES - 1, 0, "Database file does not exist, can't read.");
				} else if (!is_changed || (is_changed && confirm("Reading the database would override changes made in the current session. Read anyway? [y/N]"))) {
					read_media_arr(full_media_arr, database_path);
					selected_index = init_titles(top_panel, info_panel, COLS, filtered_media_arr, title_count, 0);
					mvprintw(LINES - 1, 0, "Database read.");
					is_changed = 0;
				}
				break;
			case 'e': // edit title
				def_prog_mode(); // pause ncurses mode
				endwin();
				full_media_arr[selected_index] = edit_title(*filtered_media_arr[selected_index]);
				is_changed = 1;
				reset_prog_mode(); // resume ncurses mode
				init_titles(top_panel, info_panel, COLS, filtered_media_arr, title_count, selected_index);
				refresh();
				break;
			case 'o': // get omdb data
				if (*api_key) {
					is_changed = process_omdb(api_key, *filtered_media_arr[selected_index-1]);
					init_titles(top_panel, info_panel, COLS, filtered_media_arr, title_count, selected_index);
					refresh();
				}
				break;
			case KEY_ENTER: // open movie
			case '\n':
				def_prog_mode(); // pause ncurses mode
				endwin();
				filtered_media_arr[selected_index]->watched = 1;
				open_video(filtered_media_arr[selected_index]->path, video_player);
				reset_prog_mode(); // resume ncurses mode
				init_titles(top_panel, info_panel, COLS, filtered_media_arr, title_count, selected_index);
				refresh();
				break;
			case '/': // search
				num_results = search(filtered_media_arr, title_count, result_indexes);
				selected_index = result_indexes[0];
				select_title(top_panel, info_panel, filtered_media_arr, selected_index, title_count); // select first result
				result_selected = 0;
				break;
			case 'n': // goto next search result
				if (num_results > 0) {
					result_selected = next_result_index(selected_index, result_indexes, num_results);
					selected_index = result_selected;
					select_title(top_panel, info_panel, filtered_media_arr, selected_index, title_count);
				} else {
					mvprintw(LINES-1, 0, "Not currently searching");
				}
				break;
			case 'N': // goto previous search result
				if (num_results > 0) {
					result_selected = prev_result_index(selected_index, result_indexes, num_results);
					selected_index = result_selected;
					select_title(top_panel, info_panel, filtered_media_arr, selected_index, title_count);
				} else {
					mvprintw(LINES-1, 0, "Not currently searching");
				}
				break;
			case 'f': // filter
				title_count = filter_titles(full_media_arr, filtered_media_arr, full_title_count);
				init_titles(top_panel, info_panel, COLS, filtered_media_arr, title_count, 0);
				selected_index = 0;
				break;
			case 't': // sort title
				switch (sort) {
					case TITLE_FORWARD:
						qsort(filtered_media_arr, title_count, sizeof(struct media *), compare_titles_reverse);
						sort = TITLE_REVERSE;
						break;
					case TITLE_REVERSE:
					default:
						qsort(filtered_media_arr, title_count, sizeof(struct media *), compare_titles);
						sort = TITLE_FORWARD;
						break;
				}
				init_titles(top_panel, info_panel, COLS, filtered_media_arr, title_count, 0);
				selected_index = 0;
				break;
			case 'y': // sort year
				switch (sort) {
					case YEAR_FORWARD:
						qsort(filtered_media_arr, title_count, sizeof(struct media *), compare_years_reverse);
						sort = YEAR_REVERSE;
						break;
					case YEAR_REVERSE:
					default:
						qsort(filtered_media_arr, title_count, sizeof(struct media *), compare_years);
						sort = YEAR_FORWARD;
						break;
				}
				init_titles(top_panel, info_panel, COLS, filtered_media_arr, title_count, 0);
				selected_index = 0;
				break;
			case 'q': // quit
				if ((is_changed && confirm("Unsaved changes have been made, quit anyway? [y/N]")) || (!is_changed && confirm("Quit popcorn? [y/N]"))) {
					endwin();
					return 0;
				} 
				break;
		}
	}

	return 0;
}

int compare_titles(const void *a, const void *b) {
	int l = tolower((**(struct media **)a).title[0]);
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

int sort() {

	return 0;
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

int prev_result_index(int current_index, int* result_indexes, int num_results) {
	int prev_result_index = -1;

	// check that there is a prev result
	for (int i = num_results-1; i >= 0 && prev_result_index == -1; i--) {
		if (result_indexes[i] < current_index) {
			prev_result_index = result_indexes[i];
		}
	}

	if (prev_result_index == -1) {
		mvprintw(LINES-1, 0, "Search hit top, continuing at bottom...");
		prev_result_index = result_indexes[num_results-1];
	}

	return prev_result_index;
}

int next_result_index(int current_index, int* result_indexes, int num_results) {
	int next_result_index = -1;

	// check that there is a later result
	for (int i = 0; i < num_results && next_result_index == -1; i++) {
		if (result_indexes[i] > current_index) {
			next_result_index = result_indexes[i];
		}
	}

	if (next_result_index == -1) {
		mvprintw(LINES-1, 0, "Search hit bottom, continuing at top...");
		next_result_index = result_indexes[0];
	}

	return next_result_index;
}

/* returns the number of matches */
int search(struct media **filtered_media_arr, int title_count, int *index_array) {
	const int QUERY_SIZE = 512;
	char query[QUERY_SIZE];
	regex_t regex;
	int matches = 0;

	request("/", query, QUERY_SIZE);

	int value = regcomp(&regex, query, 0);

	if (value == 0) {
		// regex compiled successfully
		for (int i = 0; i < title_count; i++) {
			if (regexec(&regex, filtered_media_arr[i]->title, 0, NULL, 0) == 0) {
				// match
				index_array[matches] = i;
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

int process_omdb(char* api_key, struct media *title) {
	int changed_status = 0;
	char input[50];
	request("Enter title or IMDB id: ", input, 50);

	def_prog_mode(); // pause ncurses mode
	endwin();
	changed_status = get_movie_json(input, api_key, title);
	reset_prog_mode(); // resume ncurses mode

	return changed_status;
}

int init_titles(WINDOW *top, WINDOW *info, int window_width, struct media **media_arr, int size, int index) {
	werase(top);
	display_titles(top, window_width, media_arr, size);
	select_title(top, info, media_arr, index, size);
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

int select_title(WINDOW *top_window, WINDOW *info_window, struct media **media_arr, int index, int total) {
	// highlight title on top box
	wchgat(top_window, -1, A_NORMAL, 0, NULL);
	mvwchgat(top_window, index, 0, -1, A_BOLD, 1, NULL);

	// print info in info window
	display_info(info_window, *media_arr[index], index, total);

	wrefresh(top_window);
	wrefresh(info_window);

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
