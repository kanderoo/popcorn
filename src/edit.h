struct media edit_title(struct media title);
int populate_temp_edit_file(struct media title, FILE *fp);
int open_editor(char *file_name);
struct media parse_temp_edit_file(FILE *fp);
