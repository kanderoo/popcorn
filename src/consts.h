char media_dir[200];
int title_count;

struct media {
	char title[100];
	char info[512];
	char path[300];
};

struct media media_arr[50];
// struct media *media_arr = malloc(50 * sizeof(struct media));
