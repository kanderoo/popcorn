#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include "popcorn.h"
#include "ui.h"
#include "edit.h"

int begin_ui(struct media *media_arr, char *database_path, int title_count) {
	init_ncurses();

	begin_list_stack_layout(media_arr, database_path, title_count);

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

int begin_list_stack_layout(struct media *media_arr, char *database_path, int title_count) {
	int is_changed = 0;

	// init windows
	const int TOP_PANEL_HEIGHT = (LINES - 2)/2;
	WINDOW *top_panel = newwin(TOP_PANEL_HEIGHT, COLS, 1, 0);
	WINDOW *info_panel = newwin(LINES-(TOP_PANEL_HEIGHT + 2), COLS, TOP_PANEL_HEIGHT+2, 0);
	refresh();

	// top bar
	mvprintw(0, (COLS-21)/2, "Popcorn Movie Manager");
	mvchgat(0, 0, -1, A_BOLD, 1, NULL);

	// horizontal line
	attron(COLOR_PAIR(2));
	mvhline(TOP_PANEL_HEIGHT+1, 0, 0, COLS);
	attroff(COLOR_PAIR(2));

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
				highlight_title(top_panel, info_panel, media_arr, selected_index);
				break;
			case 'k':
			case KEY_UP: // move up
				if (selected_index > 0) selected_index--;
				highlight_title(top_panel, info_panel, media_arr, selected_index);
				break;
			case 'g':
			case KEY_HOME: // go to top
				selected_index = 0;
				highlight_title(top_panel, info_panel, media_arr, selected_index);
				break;
			case 'G':
			case KEY_END: // go to bottom
				selected_index = title_count - 1;
				highlight_title(top_panel, info_panel, media_arr, selected_index);
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

int init_titles(WINDOW *top, WINDOW *info, int window_width, struct media *media_arr, int size, int index) {
	wclear(top);
	display_titles(top, window_width, media_arr, size);
	highlight_title(top, info, media_arr, index);
	return 0;
}

int display_titles(WINDOW *window, int window_width, struct media *media_arr, int size) {
	for (int i = 0; i < size; i++) {
		if (strlen(media_arr[i].title) < window_width) {
			mvwprintw(window, i, 0, media_arr[i].title);
		} else {
			mvwprintw(window, i, 0, "%.*s...", window_width-3, media_arr[i].title);
		}
	}

	return 0;
}

int display_info(WINDOW *info_window, struct media *media_arr, int index) {
	wclear(info_window);
	wprintw(info_window, media_arr[index].summary);

	return 0;
}

int highlight_title(WINDOW *top_window, WINDOW *info_window, struct media *media_arr, int index) {
	// highlight title on top box
	wchgat(top_window, -1, A_NORMAL, 0, NULL);
	mvwchgat(top_window, index, 0, -1, A_BOLD, 1, NULL);

	// print info in info window
	display_info(info_window, media_arr, index);

	wrefresh(top_window);
	wrefresh(info_window);

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
