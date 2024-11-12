#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFFER_SIZE 256
#define PIPE_NUM "/tmp/pipe_num"
#define PIPE_STR "/tmp/pipe_str"

int main() {
    // Abrir os pipes uma vez no início em modo não bloqueante
    int num_pipe = open(PIPE_NUM, O_RDONLY | O_NONBLOCK);
    int str_pipe = open(PIPE_STR, O_RDONLY | O_NONBLOCK);

    // Verificar se os pipes foram abertos com sucesso
    if (num_pipe == -1 || str_pipe == -1) {
        perror("Falha ao abrir os pipes");
        return 1;
    }

    char buffer[BUFFER_SIZE];
    while (1) {
        // Ler número do PIPE_NUM
        int num_read = read(num_pipe, buffer, BUFFER_SIZE);
        if (num_read > 0) {
            printf("Número recebido: %s\n", buffer);
        }

        // Ler trecho da música do PIPE_STR
        int str_read = read(str_pipe, buffer, BUFFER_SIZE);
        if (str_read > 0) {
            printf("Trecho recebido: %s\n", buffer);
        }

        // Pausa de 1 segundo antes de ler novamente
        sleep(1);
    }

    // Fechar os pipes quando o programa terminar
    close(num_pipe);
    close(str_pipe);
    return 0;
}
