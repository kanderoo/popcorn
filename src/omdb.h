size_t store_json(char *json_buffer, size_t size, size_t nmemb, char *destination);
int json_to_media(cJSON *json, struct media *user_media);
int print_json(cJSON *json);
int get_movie_json(char* title, char* api_key, struct media *media);
