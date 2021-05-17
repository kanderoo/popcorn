enum sort_type {
	TITLE_FORWARD,
	TITLE_REVERSE,
	YEAR_FORWARD,
	YEAR_REVERSE
};

struct ui_state {
	WINDOW *title_win;
	WINDOW *info_win;
	int title_count; // displayed titles
	int full_title_count; // all titles
	int selected_index;
	int is_changed;
	enum sort_type sort_status;
};

struct search_state {
	int num_results;
	int *result_indexes;
	int result_selected;
};

// generic ncurses functions
int begin_ui(struct media *media_arr, char *database_path, int title_count, char* api_key, char* video_player);
int init_ncurses();
void init_colors();
int confirm(char* message);
int request(char* message, char* string, int size);

// display functions
int init_titles(struct media **media_arr, int window_width, struct ui_state state);
int display_titles(WINDOW *window, int window_width, struct media **media_arr, int max);
int display_info(WINDOW *info_window, struct media title, int index, int total);
int select_title(struct media **media_arr, struct ui_state state);
int display_bottom_bar(char* text);
int display_top_bar(char* text);
int mvwprintstr_indented(WINDOW *window, int x, int y, char* string);
int filter_titles(struct media *full_media_arr, struct media **filtered_media_arr, int title_count);

// compare functions for sort
int compare_titles(const void *a, const void *b);
int compare_titles_reverse(const void *a, const void *b);
int compare_years(const void *a, const void *b);
int compare_years_reverse(const void *a, const void *b);

// ui behavior functions
int ui_save(struct media *full_media_arr, char *database_path, int title_count, int *is_changed);
int ui_read(struct media *full_media_arr, struct media **filtered_media_arr, char* database_path, struct ui_state *state);
int ui_edit(struct media **filtered_media_arr, struct ui_state *state);
int ui_omdb(struct media **filtered_media_arr, char *api_key, struct ui_state *state);
int ui_open_title(struct media **filtered_media_arr, char *video_player, struct ui_state state);
int ui_search(struct media **filtered_media_arr, struct search_state *search_state, struct ui_state *ui_state);
int ui_next_result(struct media **filtered_media_arr, struct search_state *search_state, struct ui_state *ui_state);
int ui_prev_result(struct media **filtered_media_arr, struct search_state *search_state, struct ui_state *ui_state);
int ui_filter(struct media *full_media_arr, struct media **filtered_media_arr, struct ui_state *state);
int ui_quit(int is_changed);

// sort behavior functions
int ui_sort_title_toggle(struct media **filtered_media_arr, struct ui_state *state);
int ui_sort_title(struct media **filtered_media_arr, struct ui_state *state, enum sort_type type);
int ui_sort_year_toggle(struct media **filtered_media_arr, struct ui_state *state);
int ui_sort_year(struct media **filtered_media_arr, struct ui_state *state, enum sort_type type);
