enum media_type {
	MOVIE,
	TV
};

struct media {
	char title[256];
	char summary[512];
	int year;
	int watched;
	enum media_type type;
	char path[300];
};

struct configuration {
	char media_dir[256];
	char database_path[256];
	struct media media_arr[50]; // TODO: deprecate this when moving to sqlite database
};

int set_media_dir(char *dir);
int set_media_arr(struct media *media_arr, char *directory, int recur_depth, int array_offset);
int save_media_arr(struct media *media_arr, char *store_file, int size);
int read_media_arr(struct media *media_arr, char *store_file);
int check_save_file_exists(char *file_path);
