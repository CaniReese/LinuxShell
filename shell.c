#include "shell.h"

// Arkaplanda çalışan işlemler için sinyal yöneticisi
void sigchld_handler(int signo) {
    (void)signo; // Unused parameter uyarısını kaldırır.
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("\n[PID %d] exited with status: %d\n> ", pid, WEXITSTATUS(status));
        fflush(stdout);
    }
}

// Komut satırı istemi (prompt) gösterme
void display_prompt() {
    printf("> ");
    fflush(stdout);
}

// Komutları ayırma ve parse etme
int parse_command(char *input, char **args) {
    int i = 0;
    char *token = strtok(input, " \t\n");
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
    return i;
}


// I/O Yönlendirme Kontrolü
void handle_redirection(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Error: Missing output file after '>'\n");
                return;
            }
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("Failed to open output file");
                return;
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
        } 
        else if (strcmp(args[i], "<") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Error: Missing input file after '<'\n");
                return;
            }
            int fd = open(args[i + 1], O_RDONLY);
            if (fd < 0) {
                perror("Failed to open input file");
                return;
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL;
        }
    }
}

