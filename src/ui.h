enum sort_type {
	TITLE_FORWARD,
	TITLE_REVERSE,
	YEAR_FORWARD,
	YEAR_REVERSE
};

// generic ncurses functions
int begin_ui(struct media *media_arr, char *database_path, int title_count, char* api_key, char* video_player);
int init_ncurses();
void init_colors();
int confirm(char* message);

// list_stack_layout functions
int begin_stack_layout(struct media *full_media_arr, struct media **working_media_arr, char *database_path, int title_count, char *api_key, char *video_player);
int init_titles(WINDOW *top, WINDOW *info, int window_width, struct media **media_arr, int size, int index);
int display_titles(WINDOW *window, int window_width, struct media **media_arr, int max);
int display_info(WINDOW *info_window, struct media title, int index, int total);
int select_title(WINDOW *top_window, WINDOW *info_window, struct media **media_arr, int index, int total);
int display_bottom_bar(char* text);
int display_top_bar(char* text);
int mvwprintstr_indented(WINDOW *window, int x, int y, char* string);
int process_omdb();
int request(char* message, char* string, int size);
int search(struct media **filtered_media_arr, int title_count, int *index_array);
int prev_result_index(int current_index, int* result_indexes, int num_results);
int next_result_index(int current_index, int* result_indexes, int num_results);
int filter_titles(struct media *full_media_arr, struct media **filtered_media_arr, int title_count);
int compare_titles(const void *a, const void *b);
int compare_titles_reverse(const void *a, const void *b);
int compare_years(const void *a, const void *b);
int compare_years_reverse(const void *a, const void *b);
