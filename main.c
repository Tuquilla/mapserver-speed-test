#include <stdio.h>
#include <stdint.h>
#include <curl/curl.h>
#include <time.h>
#include <windows.h>
#include <pthread.h>

#define LINESIZE 1024

const int NUM_THREADS = 2;

struct ARGS {
    char (*inputUrls)[LINESIZE];
    int startIndex;
    int endIndex;
};

size_t write_data(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total = size * nmemb;
    return total;
}

void *callMapserver(void *args) {

    struct ARGS *actual_args = args;
    float times[actual_args->endIndex - actual_args->startIndex];

    CURL * handle = curl_easy_init();
    for (int i = actual_args->startIndex; i < actual_args->endIndex; i++) {

        unsigned long long start = GetTickCount64();
        curl_easy_setopt(handle, CURLOPT_URL, actual_args->inputUrls[i]);
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &write_data);
        CURLcode success = curl_easy_perform(handle);
        unsigned long long end = GetTickCount64();

        if (success == CURLE_OK) {
            float duration = ((float) end - (float) start) / 1000;
            printf("Execution time: %fs\n", duration);
            times[i - actual_args->startIndex] = duration;
        } else {
            printf("\nCall failed with error code: %d\n", (int)success);
        }
    }
    curl_easy_cleanup(handle);
    float average_duration = 0;
    for (int i = 0; i < actual_args->endIndex - actual_args->startIndex; i++) {
        average_duration += times[i];
    }
    printf("Average duration of thread: %fs\n", average_duration / (actual_args->endIndex - actual_args->startIndex));
    return NULL;
}

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
    fptr = fopen(filepath, "r");
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
    pthread_t threads[NUM_THREADS];
    struct ARGS* argsArray[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        struct ARGS *args = malloc(sizeof *args);
        args->inputUrls = inputUrls;
        args->startIndex = (inputUrlIndex * i) / NUM_THREADS;
        args->endIndex   = (inputUrlIndex * (i + 1)) / NUM_THREADS;
        argsArray[i] = args;
        pthread_create(&threads[i], NULL, callMapserver, args);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    curl_global_cleanup();
    free(inputUrls);
    for (int i = 0; i < NUM_THREADS; i++) {
        free(argsArray[i]);
    }
    return 0;
}
