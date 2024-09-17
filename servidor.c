#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <cJSON.h>

#define BUFFER_SIZE 256
#define PIPE_NUM "/tmp/pipe_num" // FIFO para números
#define PIPE_STR "/tmp/pipe_str" // FIFO para strings

// Estrutura para acumular dados da requisição
struct data_build {
    char *ptr_char;
    size_t len_data;
};

// Inicializa a estrutura e aloca memória
void init_struct(struct data_build *access_struct) {
    access_struct->len_data = 0;
    access_struct->ptr_char = malloc(access_struct->len_data + 1);
  
    access_struct->ptr_char[0] = '\0';
}

// Callback para acumular os dados do HTTP
size_t my_callback_data(void *ptr_char, size_t size, size_t num_received, struct data_build *access_struct) {
    size_t new_len = access_struct->len_data + size * num_received;
    access_struct->ptr_char = realloc(access_struct->ptr_char, new_len + 1);
   
    memcpy(access_struct->ptr_char + access_struct->len_data, ptr_char, size * num_received);
    access_struct->ptr_char[new_len] = '\0';
    access_struct->len_data = new_len;

    return size * num_received;
}

// Obtém e seleciona um trecho da música
void get_song_extract_and_select(char *selected_song_excerpt) {
    CURL *curl_handle; //manipular o dado
    CURLcode res; //retorno da library

    struct data_build response; //acumulo
    init_struct(&response); //malloc

    char api_url[256];
    snprintf(api_url, sizeof(api_url), "https://api.vagalume.com.br/search.php?art=Zezé%%20di%%20Camargo%%20e%%20Luciano&mus=Flores%%20em%%20Vida&apikey=88f28adee8598d94334ceb6f0b32a9fd"); //formatando e armazenando a url da api

    curl_handle = curl_easy_init();
    if (curl_handle) {
        curl_easy_setopt(curl_handle, CURLOPT_URL, api_url);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, my_callback_data);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl_handle);
        if (res != CURLE_OK) {
            fprintf(stderr, "O curl_easy_perform() falhou: %s\n", curl_easy_strerror(res));
        }

        cJSON *json = cJSON_Parse(response.ptr_char);
        if (json) {
            cJSON *mus = cJSON_GetObjectItemCaseSensitive(json, "mus");
            if (cJSON_IsArray(mus)) {
                cJSON *first_song = cJSON_GetArrayItem(mus, 0);
                if (first_song) {
                    cJSON *song_text = cJSON_GetObjectItemCaseSensitive(first_song, "text");
                    if (cJSON_IsString(song_text) && (song_text->valuestring != NULL)) {
                        char *text = song_text->valuestring;
                        char *token = strtok(text, "\n\n");
                        int num_tokens = 0;
                        char *selected_token = NULL;
                        while (token != NULL) {
                            if (rand() % (++num_tokens) == 0) {
                                selected_token = token;
                            }
                            token = strtok(NULL, "\n\n");
                        }
                        if (selected_token != NULL) {
                            snprintf(selected_song_excerpt, BUFFER_SIZE, "%s", selected_token);
                        } else {
                            snprintf(selected_song_excerpt, BUFFER_SIZE, "Nenhum trecho encontrado.");
                        }
                    }
                }
            }
            cJSON_Delete(json);
        }

        free(response.ptr_char);
        curl_easy_cleanup(curl_handle);
    }
}

// Envia um número aleatório para o pipe
void send_random_num() {
    char buffer[BUFFER_SIZE];
    int num = rand() % 100 + 1;
    snprintf(buffer, BUFFER_SIZE, "%d", num);

    int pipe_fd = open(PIPE_NUM, O_WRONLY);

    write(pipe_fd, buffer, strlen(buffer) + 1);
    close(pipe_fd);
    printf("Número enviado: %d\n", num);
}

// Envia um trecho da música para o pipe
void send_song_excerpt() {
    char song_buffer[BUFFER_SIZE];
    get_song_extract_and_select(song_buffer);

    int pipe_fd = open(PIPE_STR, O_WRONLY);
    
    write(pipe_fd, song_buffer, strlen(song_buffer) + 1);
    close(pipe_fd);
    printf("Trecho enviado: %s\n", song_buffer);
}

void *thread_function() {
    while (1) {
        send_random_num();
        send_song_excerpt();
        sleep(1);
    }
    return NULL;
}

int main() {
    mkfifo(PIPE_NUM, 0666);
    mkfifo(PIPE_STR, 0666);

    pthread_t thread_pool[1];
    pthread_create(&thread_pool[0], NULL, thread_function, NULL);

    pthread_join(thread_pool[0], NULL);

    unlink(PIPE_NUM);
    unlink(PIPE_STR);

    return 0;
}
