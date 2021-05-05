struct configuration default_configuration();
int store_config(struct configuration *config);
int get_config_path(char *config_path, int buffer_size);
int parse_config(char* file_name, struct configuration *config);
int check_valid_key(char *key);
int assign_key(char *key_ptr, char *value_ptr, struct configuration *config);
