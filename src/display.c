#include<stdlib.h>
#include<ncurses.h>

char movies[10][40] = {"Movie 1", "Movie 2", "Movie 3", "Movie 4", "Movie 5", "Movie 6", "Movie 7", "Movie 8", "Movie 9", "Movie 10"};

int display_titles(WINDOW *window, int max, int boldIndex) {
	for (int i = 0; i < max; i++) {
		if (i == boldIndex) {
			wattron(window, A_BOLD);
			wprintw(window, "%s\n", movies[i]);
			wattroff(window, A_BOLD);
		} else {
			wprintw(window, "%s\n", movies[i]);
		}
	}

	return 0;
}

int quit(WINDOW *window) {
	mvprintw(LINES - 1, 0, "Quit reel [y/N]?");
	switch (getch()) {
		case 121:
		case 89:
			delwin(window);
			exit(0);
			break;
		case 110:
		case 78:
			return 0;
			break;
	}
	return 1;
}

void init_colors() {
	if (has_colors == false) {
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
	noecho();
	refresh();

	WINDOW *sidePanel = newwin(LINES, 50, 1, 0);

	attron(COLOR_PAIR(1));
	attron(A_BOLD);
	mvprintw(0, 0, "Popcorn Movie Manager");
	for (int i = 21; i < COLS; i++) {
		mvaddch(0, i, ' ');
	}
	attroff(A_BOLD);
	attron(COLOR_PAIR(2));
	mvvline(1, 50, 0, LINES);
	attroff(COLOR_PAIR(2));

	int TITLE_COUNT = 10;
	int selectedIndex = 0;

	display_titles(sidePanel, TITLE_COUNT, 0);

	wrefresh(sidePanel);

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
				quit(sidePanel);
				break;
		}
		wclear(sidePanel);
		display_titles(sidePanel, TITLE_COUNT, selectedIndex);
		wrefresh(sidePanel);
	}

	
	clear();
	endwin();

	return 0;
}
