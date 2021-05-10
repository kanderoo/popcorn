enum media_type {
	MOVIE,
	TV
};

struct media {
	char title[256];
	char summary[1024];
	char genre[32];
	int year;
	int watched;
	int runtime;
	enum media_type type;
	char **tags;
	char path[300];
};

struct configuration {
	char media_dir[256];
	char database_path[256];
};

int set_media_dir(char *dir);
int scan_media_dir(struct media *media_arr, char *directory, int recur_depth, int array_offset, int request_edits);
int save_media_arr(struct media *media_arr, char *store_file, int size);
int read_media_arr(struct media *media_arr, char *store_file);
int check_save_file_exists(char *file_path);
int linebreak_string(char *string, int line_length);
