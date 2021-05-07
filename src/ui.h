// generic ncurses functions
int begin_ui();
int init_ncurses();
void init_colors();
int confirm(char* message);

// list_stack_layout functions
int begin_list_stack_layout(struct media *media_arr, char *database_path, int title_count);
int init_titles(WINDOW *top, WINDOW *info, int window_width, struct media *media_arr, int size, int index);
int display_titles(WINDOW *window, int window_width, struct media *media_arr, int max);
int display_info(WINDOW *info_window, struct media *media_arr, int index);
int highlight_title(WINDOW *top_window, WINDOW *info_window, struct media *media_arr, int index);
