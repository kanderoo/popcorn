#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include "popcorn.h"
#include "edit.h"

struct media edit_title(struct media title) {
	char temp_name[300];
	sprintf(temp_name, "/tmp/popcorn-title-%s.tmp", title.title);
	FILE *fp;
	fp = fopen(temp_name, "w+");

	populate_temp_edit_file(title, fp);

	fclose(fp); // close file to allow editor to take over
	open_editor(temp_name);
	fp = fopen(temp_name, "r"); // reopen file

	title = parse_temp_edit_file(fp);

	fclose(fp);
	remove(temp_name); // delete temp file

	return title;
}

int populate_temp_edit_file(struct media title, FILE *fp) {
	fprintf(fp, "Title: %s\n", title.title);
	fprintf(fp, "Summary: %s\n", title.summary);
	fprintf(fp, "Genre: %s\n", title.genre);
	fprintf(fp, "Year: %d\n", title.year);
	fprintf(fp, "Watched: %d\n", title.watched);
	fprintf(fp, "Runtime: %d\n", title.runtime);
	fprintf(fp, "Path: %s\n", title.path);
	
	return 0;
}

int open_editor(char *file_name) {
	char* editor = getenv("EDITOR");

	if (!editor) {
		// no editor, default to vi
		editor = "vi";
	}

	pid_t pid = fork();

	if (pid == 0) {
		// is child process
		execlp(editor, editor, file_name, NULL);

		// the following will execute if exec fails
		fprintf(stderr, "Error: editor could not be opened.");
		exit(500);
	} else {
		// is parent, wait for editor to finish
		waitpid(pid, 0, 0);
	}

	return 0;
}

/* returns a new, populated title */
struct media parse_temp_edit_file(FILE *fp) {
	struct media new_title;
	fscanf(fp, "Title: %[^\n]\n", new_title.title);
	fscanf(fp, "Summary: %[^\n]\n", new_title.summary);
	fscanf(fp, "Genre: %[^\n]\n", new_title.genre);
	fscanf(fp, "Year: %d\n", &new_title.year);
	fscanf(fp, "Watched: %d\n", &new_title.watched);
	fscanf(fp, "Runtime: %d\n", &new_title.runtime);
	fscanf(fp, "Path: %[^\n]\n", new_title.path);

	return new_title;
}
