int store_config();
int get_config_path(char *config_path, int buffer_size);
int parse_config(char* file_name);
int check_valid_key(char *key);
int assign_key(char *key_ptr, char *value_ptr);
