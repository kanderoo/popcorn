#include <stdio.h>
#include <sqlite3.h>

int init_database() {
	sqlite3 *db;
	int rc = sqlite3_open("movies.db", &db);

	if (rc) {
		// failure
		fprintf(stderr, "Error trying to load databse: %s\n", sqlite3_errmsg(db));
		return 1;
	} 

	sqlite3_close(db);
}

int main() {
	if (init_database()) {
		return 1;
	}

	return 0;
}
