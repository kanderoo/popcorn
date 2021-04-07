#include <stdlib.h>
#include <ncurses.h>

char movies[10][40] = {"Movie 1", "Movie 2", "Movie 3", "Movie 4", "Movie 5", "Movie 6", "Movie 7", "Movie 8", "Movie 9", "Movie 10"};
char descriptions[10][50] = {"Movie 1 will knock your socks off", "Movie 2 is the highlight of the year", "Movie 3 was boring and stupid", "Movie 4 was just okay, maybe a bit predictable", "Movie 5 was a thing", "I hated Movie 6 with a passion", "Movie 7 was an awesome spectacle", "Movie 8 is the greatest thing on earth", "Movie 9 was okay", "Movie 10 sucked"};

int display_titles(WINDOW *window, int max) {
	for (int i = 0; i < max; i++) {
		wprintw(window, "%s\n", movies[i]);
	}

	return 0;
}

int display_info(WINDOW *info_win, int index) { // `index` will eventually be a file name and info will be gotten from metadata
	mvwprintw(info_win, 0, 0, "%s\n", descriptions[index]);
}

int highlight_title(WINDOW *side_window, WINDOW *info_window, int index) {
	wchgat(side_window, -1, A_NORMAL, 0, NULL);
	mvwchgat(side_window, index, 0, -1, A_BOLD, 1, NULL);
	display_info(info_window, index);
	return 0;
}

int quit(WINDOW *window) {
	mvprintw(LINES - 1, 0, "Quit popcorn? [y/N]");
	switch (getch()) {
		case 121: // Y
		case 89:  // y
			delwin(window);
			exit(0);
			break;
		case 110: // N
		case 78:  // n
			// clear prompt
			move(LINES - 1, 0);
			clrtoeol();
			return 0;
			break;
	}
	return 1;
}

void init_colors() {
	if (has_colors() == false) {
		endwin();
		printf("Your teminal does not support color. Stop living in 1972.");
		exit(1);
	}
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLUE);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
}

int main(int argc, char* argv[]) {
	// init ncurses
	initscr();
	init_colors();
	raw();
	curs_set(0);
	noecho();
	refresh();

	// init windows
	WINDOW *side_panel = newwin(LINES, 50, 1, 0);
	WINDOW *info_win = newwin(LINES, COLS - 49, 1, 51);
	wprintw(info_win, "Hello, World!");

	// top bar
	mvprintw(0, 0, "Popcorn Movie Manager");
	mvchgat(0, 0, -1, A_BOLD, 1, NULL);

	// vertical line
	attron(COLOR_PAIR(2));
	mvvline(1, 50, 0, LINES);
	attroff(COLOR_PAIR(2));

	// display titles
	int TITLE_COUNT = 10; // TODO: get this from config
	int selectedIndex = 0;

	display_titles(side_panel, TITLE_COUNT);
	highlight_title(side_panel, info_win, 0);

	wrefresh(side_panel);

	// get user input
	while (1) {
		switch (getch()) {
			case 104: // h
				break;
			case 106: // j
				if (selectedIndex < TITLE_COUNT - 1) {
					selectedIndex++;
				}
				break;
			case 107: // k
				if (selectedIndex > 0) {
					selectedIndex--;
				}
				break;
			case 108: // l
				break;
			case 103: // g
				selectedIndex = 0;
				break;
			case 71: // G
				selectedIndex = TITLE_COUNT - 1;
				break;
			case 113: // q
				quit(side_panel);
				break;
		}
		highlight_title(side_panel, info_win, selectedIndex);
		wrefresh(side_panel);
	}
	
	// cleanup
	clear();
	curs_set(1);
	refresh();
	endwin();

	return 0;
}
