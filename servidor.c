#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFFER_SIZE 100
#define THREAD_POOL_SIZE 4 
#define MAX_TASKS 10     
#define PIPE_NUM "/tmp/pipe_num"
#define PIPE_STR "/tmp/pipe_str"

int request_counter = 0;  

pthread_mutex_t lock;   
pthread_cond_t cond;   

typedef struct {
    void (*function)(void* arg);      
    void* arg;                    
} Task;

Task task_queue[MAX_TASKS];  
int task_count = 0;            

void add_task(void (*function)(void* arg), void* arg) {
    pthread_mutex_lock(&lock);
    if (task_count < MAX_TASKS) {
        task_queue[task_count].function = function;
        task_queue[task_count].arg = arg;
        task_count++;
        pthread_cond_signal(&cond);  
    }
    pthread_mutex_unlock(&lock);
}

Task get_task() {
    Task task;
    pthread_mutex_lock(&lock);
    while (task_count == 0) {  
        pthread_cond_wait(&cond, &lock);
    }
    task = task_queue[0];  
    for (int i = 0; i < task_count - 1; i++) {
        task_queue[i] = task_queue[i + 1];  
    }
    task_count--;
    pthread_mutex_unlock(&lock);
    return task;
}

void respond_with_number(void* arg) {
    char buffer[BUFFER_SIZE];
    int number = rand() % 100 + 1;  
    snprintf(buffer, BUFFER_SIZE, "%d", number);

    int fd = open(PIPE_NUM, O_WRONLY);
    write(fd, buffer, strlen(buffer) + 1);  
    close(fd);

    pthread_mutex_lock(&lock);
    request_counter++;
    printf("Número enviado: %d (Contagem de requisições: %d)\n", number, request_counter);
    pthread_mutex_unlock(&lock);
}

void respond_with_string(void* arg) {
    char* string_response = "Response String";
    
    int fd = open(PIPE_STR, O_WRONLY);
    write(fd, string_response, strlen(string_response) + 1);  
    close(fd);

    pthread_mutex_lock(&lock);
    request_counter++;
    printf("String enviada: %s (Contagem de requisições: %d)\n", string_response, request_counter);
    pthread_mutex_unlock(&lock);
}

void* thread_function(void* arg) {
    while (1) {
        Task task = get_task();  
        task.function(task.arg); 
    }
    return NULL;
}

void server() {
    mkfifo(PIPE_NUM, 0666); 
    mkfifo(PIPE_STR, 0666);

    sleep(1);

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_t thread_pool[THREAD_POOL_SIZE];
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&thread_pool[i], NULL, thread_function, NULL);
    }

    while (1) {
        add_task(respond_with_number, NULL);  
        add_task(respond_with_string, NULL);  
        sleep(1); 
    }

    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_join(thread_pool[i], NULL);
    }

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    unlink(PIPE_NUM);
    unlink(PIPE_STR);
}


int main() {
    srand(time(NULL));  
    server();         
    return 0;
}
