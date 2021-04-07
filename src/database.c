#include <stdio.h>
#include <sqlite3.h>

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

char *get_titles() {

}

int main() {
	init_database();

	return 0;
}
