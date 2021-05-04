int init_display();
int display_titles(WINDOW *window, int sidebar_width, struct media *media_arr, int max);
int display_info(WINDOW *info_window, struct media *media_arr, int index);
int highlight_title(WINDOW *side_window, WINDOW *info_window, struct media *media_arr, int index);
int quit();
void init_colors();
