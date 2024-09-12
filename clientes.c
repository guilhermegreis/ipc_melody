#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //POSIX
#include <fcntl.h>
#include <sys/stat.h>

#define BUFFER_SIZE 256
#define PIPE_NUM "/tmp/pipe_num"
#define PIPE_STR "/tmp/pipe_str"

void receive_number()
{
    char buffer[BUFFER_SIZE];
    int fd = open(PIPE_NUM, O_RDONLY);
    read(fd, buffer, BUFFER_SIZE);
    close(fd);

    printf("Número recebido: %s\n", buffer);
}

void receive_string()
{
    char buffer[BUFFER_SIZE];
    int fd = open(PIPE_STR, O_RDONLY);
    read(fd, buffer, BUFFER_SIZE);
    close(fd);

    printf("Música recebida: %s\n", buffer);
}

int main()
{
    while (1)
    {
        receive_number();
        receive_string();
        sleep(1);
    }
    return 0;
}
