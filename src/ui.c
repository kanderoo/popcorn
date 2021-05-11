#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <cjson/cJSON.h>
#include "popcorn.h"
#include "ui.h"
#include "edit.h"
#include "omdb.h"

int begin_ui(struct media *media_arr, char *database_path, int title_count, char* api_key, char* video_player) {
	init_ncurses();
	display_top_bar("Popcorn Movie Manager");
	begin_stack_layout(media_arr, database_path, title_count, api_key, video_player);

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

int begin_stack_layout(struct media *media_arr, char *database_path, int title_count, char *api_key, char *video_player) {
	int is_changed = 0;

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
	int selected_index = init_titles(top_panel, info_panel, COLS, media_arr, title_count, 0);

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
				select_title(top_panel, info_panel, media_arr, selected_index, title_count);
				break;
			case 'k':
			case KEY_UP: // move up
				if (selected_index > 0) selected_index--;
				select_title(top_panel, info_panel, media_arr, selected_index, title_count);
				break;
			case 'g':
			case KEY_HOME: // go to top
				selected_index = 0;
				select_title(top_panel, info_panel, media_arr, selected_index, title_count);
				break;
			case 'G':
			case KEY_END: // go to bottom
				selected_index = title_count - 1;
				select_title(top_panel, info_panel, media_arr, selected_index, title_count);
				break;
			case 's': // save media array
				if (confirm("Save current media? [Y/n]")) {
					save_media_arr(media_arr, database_path, title_count);
					mvprintw(LINES - 1, 0, "Saved!");
					is_changed = 0;
				}
				break;
			case 'r': // read media array
				if (!check_save_file_exists(database_path)) {
					mvprintw(LINES - 1, 0, "Database file does not exist, can't read.");
				} else if (!is_changed || (is_changed && confirm("Reading the database would override changes made in the current session. Read anyway? [y/N]"))) {
					read_media_arr(media_arr, database_path);
					selected_index = init_titles(top_panel, info_panel, COLS, media_arr, title_count, 0);
					mvprintw(LINES - 1, 0, "Database read.");
					is_changed = 0;
				}
				break;
			case 'e': // edit title
				def_prog_mode(); // pause ncurses mode
				endwin();
				media_arr[selected_index] = edit_title(media_arr[selected_index]);
				is_changed = 1;
				reset_prog_mode(); // resume ncurses mode
				init_titles(top_panel, info_panel, COLS, media_arr, title_count, selected_index);
				refresh();
				break;
			case 'o':
				if (*api_key) {
					is_changed = process_omdb(api_key, media_arr[selected_index-1]);
					init_titles(top_panel, info_panel, COLS, media_arr, title_count, selected_index);
					refresh();
				}
				break;
			case KEY_ENTER:
			case '\n':
				def_prog_mode(); // pause ncurses mode
				endwin();
				open_video(media_arr[selected_index].path, video_player);
				reset_prog_mode(); // resume ncurses mode
				init_titles(top_panel, info_panel, COLS, media_arr, title_count, selected_index);
				refresh();
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

int init_titles(WINDOW *top, WINDOW *info, int window_width, struct media *media_arr, int size, int index) {
	wclear(top);
	display_titles(top, window_width, media_arr, size);
	select_title(top, info, media_arr, index, size);
	return 0;
}

int display_titles(WINDOW *window, int window_width, struct media *media_arr, int size) {
	for (int i = 0; i < size; i++) {
		if (strlen(media_arr[i].title) < window_width) {
			mvwprintw(window, i, 1, "%s (%d)", media_arr[i].title, media_arr[i].year);
			mvwprintw(window, i, COLS-6, "%02d:%02d", media_arr[i].runtime / 60, media_arr[i].runtime % 60);
		} else {
			mvwprintw(window, i, 1, "%.*s...", window_width-3, media_arr[i].title);
		}
	}

	return 0;
}

int display_info(WINDOW *info_window, struct media title, int index, int total) {
	wclear(info_window);

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

int select_title(WINDOW *top_window, WINDOW *info_window, struct media *media_arr, int index, int total) {
	// highlight title on top box
	wchgat(top_window, -1, A_NORMAL, 0, NULL);
	mvwchgat(top_window, index, 0, -1, A_BOLD, 1, NULL);

	// print info in info window
	display_info(info_window, media_arr[index], index, total);

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
