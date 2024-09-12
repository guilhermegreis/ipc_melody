#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> // POSIX
#include <time.h>
#include <string.h>
#include <fcntl.h>     // manipular arquivos
#include <sys/stat.h>  // manipular permissão e propriedade
#include <curl/curl.h> // transferencias de dados url e manipulação http
#include "cJSON.h"     // cria, manipula e parsea o JSON

#define BUFFER_SIZE 256
#define THREAD_POOL_SIZE 4
#define MAX_TASKS 10             // limitar o numero de tarefas
#define PIPE_NUM "/tmp/pipe_num" // fifo
#define PIPE_STR "/tmp/pipe_str" // fifo

int request_counter = 0;

// estrutura e gerencia de fila de tarefas
typedef struct Task
{
    void (*task_function)();
} Task;

Task task_queue[MAX_TASKS];
int task_count = 0;

pthread_mutex_t mutex_queue;
pthread_cond_t cond_queue;

struct string
{
    char *ptr;
    size_t len;
};

void init_string(struct string *s)
{
    s->len = 0;
    s->ptr = malloc(s->len + 1);
    if (s->ptr == NULL)
    {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

// callback do libcurl, acumular o que recebe da requisição
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);
    if (s->ptr == NULL)
    {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size * nmemb;
}

// buscar info da musica, inicializar e fazer as coisas relacionadas a musica msm
void get_song_from_vagalume(char *song_buffer)
{
    CURL *curl;
    CURLcode res;

    struct string response;
    init_string(&response);

    char url[256];
    snprintf(url, sizeof(url), "https://api.vagalume.com.br/search.php?art=Zezé%%20di%%20Camargo%%20e%%20Luciano&mus=Flores%%20em%%20Vida&apikey=88f28adee8598d94334ceb6f0b32a9fd");

    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        printf("Resposta da API: %s\n", response.ptr);

        cJSON *json = cJSON_Parse(response.ptr);
        if (json == NULL)
        {
            fprintf(stderr, "Erro ao parsear JSON.\n");
        }
        else
        {
            cJSON *mus = cJSON_GetObjectItemCaseSensitive(json, "mus");
            if (cJSON_IsArray(mus))
            {
                cJSON *first_song = cJSON_GetArrayItem(mus, 0);
                if (first_song)
                {
                    cJSON *song_text = cJSON_GetObjectItemCaseSensitive(first_song, "text");
                    if (cJSON_IsString(song_text) && (song_text->valuestring != NULL))
                    {
                        // dividir a letra em trechos
                        char *text = song_text->valuestring;
                        char *token = strtok(text, "\n\n"); // divide em trechos por parágrafo

                        // seleciona um trecho aleatório
                        int num_tokens = 0;
                        char *selected_token = NULL;
                        while (token != NULL)
                        {
                            if (rand() % (++num_tokens) == 0)
                            {
                                selected_token = token;
                            }
                            token = strtok(NULL, "\n\n");
                        }

                        // enviar o trecho selecionado
                        if (selected_token != NULL)
                        {
                            snprintf(song_buffer, BUFFER_SIZE, "%s", selected_token);
                        }
                        else
                        {
                            snprintf(song_buffer, BUFFER_SIZE, "Nenhum trecho encontrado.");
                        }
                    }
                }
            }
            cJSON_Delete(json);
        }

        free(response.ptr);
        curl_easy_cleanup(curl);
    }
}

// gerar num aleatorio, pipe FIFO e atualização do contador de request
void respond_with_number()
{
    char buffer[BUFFER_SIZE];
    int number = rand() % 100 + 1;
    snprintf(buffer, BUFFER_SIZE, "%d", number);

    int fd = open(PIPE_NUM, O_WRONLY);
    write(fd, buffer, strlen(buffer) + 1);
    close(fd);

    pthread_mutex_lock(&mutex_queue);
    request_counter++;
    printf("Número enviado: %d (Número de requisições: %d)\n", number, request_counter);
    pthread_mutex_unlock(&mutex_queue);
}

// escrever o trecho da música, pipe FIFO e atualização do request também
void respond_with_string()
{
    char song_buffer[BUFFER_SIZE];

    get_song_from_vagalume(song_buffer);

    int fd = open(PIPE_STR, O_WRONLY);
    write(fd, song_buffer, strlen(song_buffer) + 1);
    close(fd);

    pthread_mutex_lock(&mutex_queue);
    request_counter++;
    printf("Música enviada: %s (Contagem de requisições: %d)\n", song_buffer, request_counter);
    pthread_mutex_unlock(&mutex_queue);
}

void execute_task(Task *task)
{
    task->task_function();
}

void submit_task(Task task)
{
    pthread_mutex_lock(&mutex_queue);
    if (task_count < MAX_TASKS)
    {
        task_queue[task_count] = task;
        task_count++;
        pthread_cond_signal(&cond_queue);
    }
    pthread_mutex_unlock(&mutex_queue);
}

void *thread_function(void *args)
{
    while (1)
    {
        Task task;
        pthread_mutex_lock(&mutex_queue);
        while (task_count == 0)
        {
            pthread_cond_wait(&cond_queue, &mutex_queue);
        }
        task = task_queue[0];
        for (int i = 0; i < task_count - 1; i++)
        {
            task_queue[i] = task_queue[i + 1];
        }
        task_count--;
        pthread_mutex_unlock(&mutex_queue);
        execute_task(&task);
    }
    return NULL;
}

void server()
{
    mkfifo(PIPE_NUM, 0666);
    mkfifo(PIPE_STR, 0666);

    pthread_mutex_init(&mutex_queue, NULL);
    pthread_cond_init(&cond_queue, NULL);

    pthread_t thread_pool[THREAD_POOL_SIZE];
    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        pthread_create(&thread_pool[i], NULL, thread_function, NULL);
    }

    while (1)
    {
        Task task_number = {.task_function = respond_with_number};
        submit_task(task_number);

        Task task_string = {.task_function = respond_with_string};
        submit_task(task_string);

        sleep(1);
    }

    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        pthread_join(thread_pool[i], NULL);
    }

    pthread_mutex_destroy(&mutex_queue);
    pthread_cond_destroy(&cond_queue);

    unlink(PIPE_NUM);
    unlink(PIPE_STR);
}

int main()
{
    srand(time(NULL));
    server();
    return 0;
}