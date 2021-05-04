#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include "popcorn.h"
#include "display.h"

int init_display(struct media *media_arr, int title_count) {
	// init ncurses
	initscr();
	noecho();
	cbreak();
	curs_set(0);
	init_colors();

	// init windows
	const int SIDE_PANEL_WIDTH = 50;
	WINDOW *side_panel = newwin(LINES - 2, SIDE_PANEL_WIDTH, 1, 0);
	WINDOW *info_panel = newwin(LINES - 2, COLS-(SIDE_PANEL_WIDTH + 2), 1, 52);
	refresh();

	// top bar
	mvprintw(0, 0, "Popcorn Movie Manager");
	mvchgat(0, 0, -1, A_BOLD, 1, NULL);

	// vertical line
	attron(COLOR_PAIR(2));
	mvvline(1, SIDE_PANEL_WIDTH, 0, LINES - 2);
	attroff(COLOR_PAIR(2));

	// display titles
	int selectedIndex = 0;

	// sidebar setup
	display_titles(side_panel, SIDE_PANEL_WIDTH, media_arr, title_count);
	highlight_title(side_panel, info_panel, media_arr, 0);

	// get user input
	while (1) {
		switch (getch()) {
			case 'h':
				break;
			case 'j':
				if (selectedIndex < title_count - 1) {
					selectedIndex++;
				}
				break;
			case 'k':
				if (selectedIndex > 0) {
					selectedIndex--;
				}
				break;
			case 'l':
				break;
			case 'g':
				selectedIndex = 0;
				break;
			case 'G':
				selectedIndex = title_count - 1;
				break;
			case 'q':
				quit();
				break;
		}
		highlight_title(side_panel, info_panel, media_arr, selectedIndex);
	}
	
	endwin(); // shouldn't ever get executed, but here for safety
	return 0;
}

int display_titles(WINDOW *window, int sidebar_width, struct media *media_arr, int size) {
	for (int i = 0; i < size; i++) {
		if (strlen(media_arr[i].title) < sidebar_width) {
			wprintw(window, "%s\n", media_arr[i].title);
		} else {
			wprintw(window, "%.*s...\n", sidebar_width-3, media_arr[i].title);
		}
	}

	return 0;
}

int display_info(WINDOW *info_window, struct media *media_arr, int index) {
	wclear(info_window);
	wprintw(info_window, media_arr[index].info);

	return 0;
}

int highlight_title(WINDOW *side_window, WINDOW *info_window, struct media *media_arr, int index) {
	// highlight title on sidebar
	wchgat(side_window, -1, A_NORMAL, 0, NULL);
	mvwchgat(side_window, index, 0, -1, A_BOLD, 1, NULL);

	// print info in info window
	display_info(info_window, media_arr, index);

	wrefresh(side_window);
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

int quit() {
	mvprintw(LINES - 1, 0, "Quit popcorn? [y/N]");
	switch (getch()) {
		case 121: // Y
		case 89:  // y
			endwin();
			exit(0);
			break;
		case 110: // N
		case 78:  // n
		default:
			// clear prompt
			move(LINES - 1, 0);
			clrtoeol();
			return 0;
			break;
	}
	return 1;
}
