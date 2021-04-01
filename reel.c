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
	mvprintw(57, 0, "Quit reel [y/N]?");
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

int main(int argc, char* argv[]) {
	// init ncurses
	initscr();
	raw();
	noecho();
	refresh();

	WINDOW *sidePanel = newwin(LINES, 50, 0, 0);

	mvvline(0, 50, 0, LINES);

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

	
	endwin();

	return 0;
}
