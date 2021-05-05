struct media {
	char title[256];
	char info[512];
	char path[300];
};

struct configuration {
	char media_dir[256];
	char database_path[256];
	struct media media_arr[50]; // TODO: this will be deprecated when moved to sqlite database
};

int set_media_dir(char *dir);
int set_media_arr(struct media *media_arr, char *directory, int recur_depth, int array_offset);
int store_media_arr(struct media *media_arr, char *store_file, int size);
int read_media_arr(struct media *media_arr, char *store_file);
