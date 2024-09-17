#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //POSIX
#include <fcntl.h>
#include <sys/stat.h>

#define BUFFER_SIZE 256
#define PIPE_NUM "/tmp/pipe_num"
#define PIPE_STR "/tmp/pipe_str"

//ler o numero recebido do servidor
void receive_number()
{
    char buffer[BUFFER_SIZE];
    int opened_pipe = open(PIPE_NUM, O_RDONLY);
    read(opened_pipe, buffer, BUFFER_SIZE);
    close(opened_pipe);

    printf("Número recebido: %s\n", buffer);
}

//ler a string recebida do servidor
void receive_string()
{
    char buffer[BUFFER_SIZE];
    int opened_pipe = open(PIPE_STR, O_RDONLY);
    read(opened_pipe, buffer, BUFFER_SIZE);
    close(opened_pipe);

    printf("Trecho recebida: %s\n", buffer);
}

int main()
{
    //bem bolado pra fazer a alternancia entre o num e a str e escrever no terminal RUNTIME
    while (1)
    {
        receive_number();
        receive_string();
        sleep(1);
    }
    return 0;
}
