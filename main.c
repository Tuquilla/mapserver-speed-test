#include <stdio.h>
#include <stdint.h>
#include <curl/curl.h>
#include <time.h>
#include <windows.h>

size_t write_data(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total = size * nmemb;
    return total;
}

int LINESIZE = 1024;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("No filepath given");
        return 1;
    }
    const char *filepath = argv[1];

    // Get lines numbers from input file
    FILE *fptr = fopen(filepath, "r");
    int lines = 0;
    if (fptr != NULL) {
        char line[1024];
        while (fgets(line, sizeof(line), fptr)) {
           lines++;
        }
        fclose(fptr);
    }
    printf("Lines: %d\n", lines);
    char (*inputUrls)[LINESIZE] = malloc(lines * sizeof(*inputUrls));
    int inputUrlIndex = 0;

    // Read input file
    fptr = fopen("input_url.txt", "r");
    if (fptr != NULL) {
        char line[LINESIZE];
        while (fgets(line, sizeof(line), fptr)) {
            line[strcspn(line, "\n")] = '\0';
            strncpy(inputUrls[inputUrlIndex], line, sizeof(inputUrls[inputUrlIndex]));
            inputUrlIndex++;
        }
        fclose(fptr);
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    for (int i = 0; i < inputUrlIndex; i++) {
        CURL * handle = curl_easy_init();
        unsigned long long start = GetTickCount64();
        curl_easy_setopt(handle, CURLOPT_URL, inputUrls[i]);
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &write_data);
        CURLcode success = curl_easy_perform(handle);
        unsigned long long end = GetTickCount64();

        if (success == CURLE_OK) {
            printf("\nCall successfull\n");
            printf("Execution time: %fs", ((float) end - (float)start) / 1000);
        } else {
            printf("\nCall failed with error code: %d\n", (int)success);
        }
    }

    free(inputUrls);
    return 0;
}
