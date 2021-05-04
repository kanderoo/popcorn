struct media {
	char title[100];
	char info[512];
	char path[300];
};

int set_media_dir(char *dir);
int set_media_arr(char *directory, int recur_depth, int array_offset);
