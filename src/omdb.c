#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <stdlib.h>
#include "popcorn.h"
#include "omdb.h"

size_t store_json(char *json_buffer, size_t size, size_t nmemb, char *destination) {
	size_t realsize = size * nmemb;

	strncat(destination, json_buffer, (2048-strlen(json_buffer))); // 2048 is destination buffer size

	return realsize;
}

int json_to_media(cJSON *json, struct media *user_media) {
	cJSON *title = cJSON_GetObjectItemCaseSensitive(json, "Title");
	if (cJSON_IsString(title) && (title->valuestring != NULL)) {
		strncpy(user_media->title, title->valuestring, 256);
	}

	cJSON *summary = cJSON_GetObjectItemCaseSensitive(json, "Plot");
	if (cJSON_IsString(summary) && (summary->valuestring != NULL)) {
		strncpy(user_media->summary, summary->valuestring, 1024);
	}

	
	cJSON *genre = cJSON_GetObjectItemCaseSensitive(json, "Genre");
	if (cJSON_IsString(genre) && (genre->valuestring != NULL)) {
		strncpy(user_media->genre, genre->valuestring, 64);
	}

	cJSON *year = cJSON_GetObjectItemCaseSensitive(json, "Year");
	if (cJSON_IsString(year) && (year->valuestring != NULL)) {
		user_media->year = strtol(year->valuestring, NULL, 10);
	}

	cJSON *runtime = cJSON_GetObjectItemCaseSensitive(json, "Runtime");
	if (cJSON_IsString(runtime) && (runtime->valuestring != NULL)) {
		user_media->runtime = strtol(runtime->valuestring, NULL, 10);
	}

	return 0;
}

int print_json(cJSON *json) {
	printf("OMDB Entry:\n");
	printf("===========\n\n");
	cJSON *title = cJSON_GetObjectItemCaseSensitive(json, "Title");
	if (cJSON_IsString(title) && (title->valuestring != NULL)) {
		printf("Title: %s\n", title->valuestring);
	}

	cJSON *summary = cJSON_GetObjectItemCaseSensitive(json, "Plot");
	if (cJSON_IsString(summary) && (summary->valuestring != NULL)) {
		printf("Summary: %s\n", summary->valuestring);
	}

	
	cJSON *genre = cJSON_GetObjectItemCaseSensitive(json, "Genre");
	if (cJSON_IsString(genre) && (genre->valuestring != NULL)) {
		printf("Genre: %s\n", genre->valuestring);
	}

	cJSON *year = cJSON_GetObjectItemCaseSensitive(json, "Year");
	if (cJSON_IsString(year) && (year->valuestring != NULL)) {
		printf("Year: %s\n", year->valuestring);
	}

	cJSON *runtime = cJSON_GetObjectItemCaseSensitive(json, "Runtime");
	if (cJSON_IsString(runtime) && (runtime->valuestring != NULL)) {
		printf("Runtime: %s\n", runtime->valuestring);
	}

	return 0;
}

int get_movie_json(char* title, char* api_key, struct media *media) {
	int changed_status = 0;
	CURL *curl = curl_easy_init();

	if (curl) {
		CURLcode res;
		char url[1024];
		char json_result[2048];

		title = curl_easy_escape(curl, title, 0);
		if (title[0] == 't' && title[1] == 't') {
			// is imdb id
			sprintf(url, "https://www.omdbapi.com/?plot=full&apikey=%s&i=%s", api_key, title);
		} else {
			// is title
			sprintf(url, "https://www.omdbapi.com/?plot=full&apikey=%s&t=%s", api_key, title);
		}

		curl_easy_setopt(curl, CURLOPT_WRITEDATA, json_result);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, store_json);
		curl_easy_setopt(curl, CURLOPT_URL, url);
		res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			fprintf(stderr, "Curl failed: %s\n", curl_easy_strerror(res));
		}

		cJSON *json = cJSON_Parse(json_result);

		if (json == NULL) {
			const char *error_ptr = cJSON_GetErrorPtr();
			if (error_ptr != NULL) {
				fprintf(stderr, "Error before: %s\n", error_ptr);
			}
			return 1;
		}

		print_json(json);
		printf("\nOMDB entry found, would you like to use it? ");
		char response = getchar();
		while ((getchar()) != '\n'); // consume newline
		switch(response) {
			case 'y':
			case 'Y':
				changed_status = 1;
				json_to_media(json, media);
				break;
			case 'n':
			case 'N':
			default:
				changed_status = 0;
				break;
		}

		cJSON_Delete(json);
		curl_free(title);
		curl_easy_cleanup(curl);
	}

	curl_global_cleanup();

	return changed_status;
}

