#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 100

int pipe_num[2];
int pipe_str[2];
int request_counter = 0;

pthread_mutex_t lock;

void *respond_with_number(void *arg)
{
    char buffer[BUFFER_SIZE];
    while (1)
    {
        int number = rand() % 100 + 1;
        snprintf(buffer, BUFFER_SIZE, "%d", number);
        write(pipe_num[1], buffer, strlen(buffer) + 1);

        pthread_mutex_lock(&lock);
        request_counter++;
        printf("Número enviado: %d (Contagem de requisições: %d)\n", number, request_counter);
        pthread_mutex_unlock(&lock);
        sleep(1);
    }
}

void *respond_with_string(void *arg)
{
    char buffer[BUFFER_SIZE];
    while (1)
    {
        char *string_response = "Response String";
        write(pipe_str[1], string_response, strlen(string_response) + 1);

        pthread_mutex_lock(&lock);
        request_counter++;
        printf("String enviada: %s (Contagem de requisições: %d)\n", string_response, request_counter);
        pthread_mutex_unlock(&lock);
        sleep(1);
    }
}

void server()
{
    pipe(pipe_num);
    pipe(pipe_str);

    pthread_mutex_init(&lock, NULL);

    pthread_t num_threads[2];
    for (int i = 0; i < 2; i++)
    {
        pthread_create(&num_threads[i], NULL, respond_with_number, NULL);
    }

    pthread_t str_threads[2];
    for (int i = 0; i < 2; i++)
    {
        pthread_create(&str_threads[i], NULL, respond_with_string, NULL);
    }

    char buffer[BUFFER_SIZE];
    while (1)
    {
        if (read(pipe_num[0], buffer, BUFFER_SIZE) > 0)
        {
            printf("Cliente Número: %s\n", buffer);
        }
        if (read(pipe_str[0], buffer, BUFFER_SIZE) > 0)
        {
            printf("Cliente String: %s\n", buffer);
        }
    }

    pthread_mutex_destroy(&lock);
}

int main()
{
    srand(time(NULL));
    server();
    return 0;
}
