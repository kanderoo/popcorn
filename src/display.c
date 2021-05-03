#include <stdlib.h>
#include <ncurses.h>

char movies[10][40] = {"Movie 1", "Movie 2", "Movie 3", "Movie 4", "Movie 5", "Movie 6", "Movie 7", "Movie 8", "Movie 9", "Movie 10"};
char descriptions[10][50] = {"Movie 1 will knock your socks off", "Movie 2 is the highlight of the year", "Movie 3 was boring and stupid", "Movie 4 was just okay, maybe a bit predictable", "Movie 5 was a thing", "I hated Movie 6 with a passion", "Movie 7 was an awesome spectacle", "Movie 8 is the greatest thing on earth", "Movie 9 was okay", "Movie 10 sucked"};

int display_titles(WINDOW *window, int max);
int display_info(WINDOW *info_window, int index);
int highlight_title(WINDOW *side_window, WINDOW *info_window, int index);
int quit();
void init_colors();

int main(int argc, char* argv[]) {
	// init ncurses
	initscr();
	noecho();
	cbreak();
	curs_set(0);
	init_colors();

	// init windows
	WINDOW *side_panel = newwin(LINES - 2, 50, 1, 0);
	WINDOW *info_panel = newwin(LINES - 2, COLS-52, 1, 52);
	refresh();

	// top bar
	mvprintw(0, 0, "Popcorn Movie Manager");
	mvchgat(0, 0, -1, A_BOLD, 1, NULL);

	// vertical line
	attron(COLOR_PAIR(2));
	mvvline(1, 50, 0, LINES - 2);
	attroff(COLOR_PAIR(2));

	// display titles
	int TITLE_COUNT = 10; // TODO: get this from config
	int selectedIndex = 0;

	// sidebar setup
	display_titles(side_panel, TITLE_COUNT);
	highlight_title(side_panel, info_panel, 0);

	// get user input
	while (1) {
		switch (getch()) {
			case 'h':
				break;
			case 'j':
				if (selectedIndex < TITLE_COUNT - 1) {
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
				selectedIndex = TITLE_COUNT - 1;
				break;
			case 'q':
				quit();
				break;
		}
		highlight_title(side_panel, info_panel, selectedIndex);
	}
	
	endwin(); // shouldn't ever get executed, but here for safety
	return 0;
}

int display_titles(WINDOW *window, int max) {
	for (int i = 0; i < max; i++) {
		wprintw(window, "%s\n", movies[i]);
	}

	return 0;
}

int display_info(WINDOW *info_window, int index) { // `index` will eventually be a file name and info will be gotten from metadata
	wclear(info_window);
	wprintw(info_window, descriptions[index]);

	return 0;
}

int highlight_title(WINDOW *side_window, WINDOW *info_window, int index) {
	// highlight title on sidebar
	wchgat(side_window, -1, A_NORMAL, 0, NULL);
	mvwchgat(side_window, index, 0, -1, A_BOLD, 1, NULL);

	// print info in info window
	display_info(info_window, index);

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
