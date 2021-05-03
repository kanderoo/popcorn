#include <stdio.h>
#include <sqlite3.h>
#include <dirent.h>
#include <string.h>
#include "consts.h"

int init_database() {
	sqlite3 *db;
	int rc = sqlite3_open("data/movies.db", &db);
	char *sql;
	char *z_err_msg = 0;

	if (rc) {
		// failure
		fprintf(stderr, "Error trying to load databse: %s\n", sqlite3_errmsg(db));
		return 1;
	} 

	sql = "CREATE TABLE MOVIES";

	rc = sqlite3_exec(db, sql, NULL, 0, z_err_msg);

	if (!check_sqlite(rc, z_err_msg));
		printf("Table created successfully");

	sqlite3_close(db);

	return 0;
}

int check_sqlite(int rc, char **errmsg) {
	if (rc == SQLITE_OK) {
		return 0;
	}

	fprintf(stderr, "SQL error: %s\n", &errmsg);
	return -1;
}

int *store_titles(char *media_dir) {
	// TODO: put things in the database, but for now just store them in a media array in consts.h
	DIR *dir_ptr;
	struct dirent *entry;
	dir_ptr = opendir(media_dir);

	if (dir_ptr != NULL) {
		entry = readdir(dir_ptr);
		for (int i = 0; entry != NULL; i++, entry = readdir(dir_ptr)) {
			struct media title;

			strncpy(title.title, entry->d_name, 50);
			sprintf(title.info, "This is a description for %s", title.title);
			sprintf(title.rel_path, "Relative Path bro");

			entry = readdir(dir_ptr);
			media_arr[i] = title;
		}
	} else {
		fprintf(stderr, "Error opening media directory \"%s\"", media_dir);
		return 1;
	}

	return 0;
}

int main() {
	init_database();

	return 0;
}
