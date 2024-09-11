#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFFER_SIZE 100
#define PIPE_NUM "/tmp/pipe_num"
#define PIPE_STR "/tmp/pipe_str"

void client() {
    char buffer[BUFFER_SIZE];

    while (1) {
        printf("Requesting Number...\n");
        int fd_num = open(PIPE_NUM, O_RDONLY);
        read(fd_num, buffer, BUFFER_SIZE); 
        printf("Número recebido: %s\n", buffer);
        close(fd_num);

        printf("Requesting String...\n");
        int fd_str = open(PIPE_STR, O_RDONLY);
        read(fd_str, buffer, BUFFER_SIZE);
        printf("String recebida: %s\n", buffer);
        close(fd_str);

        sleep(1);
    }
}

int main() {
    client();
    return 0;
}
