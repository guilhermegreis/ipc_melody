#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 100

int pipe_num[2];
int pipe_str[2];

void *client_request_numbers(void *arg)
{
    char buffer[BUFFER_SIZE];
    while (1)
    {
        printf("Requesting Number...\n");
        read(pipe_num[0], buffer, BUFFER_SIZE);
        printf("Received Number: %s\n", buffer);
        sleep(2);
    }
}

void *client_request_strings(void *arg)
{
    char buffer[BUFFER_SIZE];
    while (1)
    {
        printf("Requesting String...\n");
        read(pipe_str[0], buffer, BUFFER_SIZE);
        printf("Received String: %s\n", buffer);
        sleep(2);
    }
}

int main()
{
    pipe(pipe_num);
    pipe(pipe_str);

    pthread_t client_number, client_string;
    pthread_create(&client_number, NULL, client_request_numbers, NULL);
    pthread_create(&client_string, NULL, client_request_strings, NULL);

    pthread_join(client_number, NULL);
    pthread_join(client_string, NULL);

    return 0;
}
