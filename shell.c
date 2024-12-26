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

// Pipe (Boru) ile Komut Zinciri
void handle_pipe(char *input) {
    char *commands[10]; // Maksimum 10 pipe zinciri
    int num_pipes = 0;

    // Komutları Pipe ile böl ve boşlukları temizle
    char *token = strtok(input, "|");
    while (token != NULL) {
        while (*token == ' ') token++; // Başlangıçtaki boşlukları kaldır
        commands[num_pipes++] = token;
        token = strtok(NULL, "|");
    }

    // Eksik komut kontrolü
    if (num_pipes < 2 || commands[1] == NULL || strlen(commands[1]) == 0) {
        fprintf(stderr, "Pipe error: Missing second command after '|'\n");
        return;
    }

    // DEBUG: Ayrıştırılan komutları kontrol et
//    printf("DEBUG: num_pipes = %d, commands[0] = %s, commands[1] = %s\n", 
//           num_pipes, 
//           commands[0], 
//           num_pipes > 1 ? commands[1] : "NULL");

    int pipefds[2 * (num_pipes - 1)];

    // Pipe oluştur
    for (int i = 0; i < num_pipes - 1; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("Pipe failed");
            return;
        }
    }

    for (int i = 0; i < num_pipes; i++) {
        pid_t pid = fork();
        if (pid == 0) { // Çocuk süreç
            if (i != 0) {
                dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
            }
            if (i != num_pipes - 1) {
                dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
            }

            for (int j = 0; j < 2 * (num_pipes - 1); j++) {
                close(pipefds[j]);
            }

            char *args[MAX_ARGS];
            parse_command(commands[i], args);
            handle_redirection(args); // Yönlendirme kontrolü
            execvp(args[0], args);
            perror("Command execution failed");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < 2 * (num_pipes - 1); i++) {
        close(pipefds[i]);
    }

    for (int i = 0; i < num_pipes; i++) {
        wait(NULL);
    }
}



void handle_semicolon(char *input) {
    char *commands[10]; // Maksimum 10 komut
    int num_commands = 0;

    // Noktalı virgül ile komutları böl
    char *token = strtok(input, ";");
    while (token != NULL) {
        while (*token == ' ') token++; // Başlangıçtaki boşlukları temizle
        commands[num_commands++] = token;
        token = strtok(NULL, ";");
    }

    for (int i = 0; i < num_commands; i++) {
        char *args[MAX_ARGS];
        int background = 0;

        // Arkaplan komutlarını kontrol et
        if (strchr(commands[i], '&')) {
            background = 1;
            *strchr(commands[i], '&') = '\0';
        }

        // Komutları ayrıştır
        parse_command(commands[i], args);

        if (args[0] == NULL) {
            continue;
        }

        execute_command(args, background);
    }
}

void handle_background_commands(char *input) {
    char *commands[10]; // Maksimum 10 arkaplan komut
    int num_commands = 0;

    // Arkaplan komutlarını ayır
    char *token = strtok(input, "&");
    while (token != NULL) {
        while (*token == ' ') token++; // Boşlukları temizle
        commands[num_commands++] = token;
        token = strtok(NULL, "&");
    }

    for (int i = 0; i < num_commands; i++) {
        char *args[MAX_ARGS];
        parse_command(commands[i], args);

        if (args[0] == NULL) {
            continue;
        }

        execute_command(args, 1); // Arkaplanda çalıştır
    }
}


// Tekli komut çalıştırma
void execute_command(char **args, int background) {
    if (args[0] == NULL) {
        return;
    }

    if (strcmp(args[0], "quit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    pid_t pid = fork();
    if (pid == 0) { // Çocuk süreç
        handle_redirection(args); // Yönlendirme kontrolü
        execvp(args[0], args);
        perror("Error executing command");
        exit(EXIT_FAILURE);
    } else if (pid > 0) { // Ana süreç
        if (!background) {
            waitpid(pid, NULL, 0); // Ön plan süreci bekle
        } else {
            printf("[PID %d] running in background\n", pid);
        }
    } else {
        perror("Fork failed");
    }
}


// Shell Ana Döngüsü
int main() {
    char input[MAX_CMD_LEN];
    char *args[MAX_ARGS];

    signal(SIGCHLD, sigchld_handler); // Arkaplan süreçleri kontrol et

    while (1) {
        display_prompt();

        if (!fgets(input, MAX_CMD_LEN, stdin)) {
            break;
        }

        // Arkaplan Komut Kontrolü
        if (strchr(input, '&')) {
            handle_background_commands(input);
        }
        // Noktalı Virgül Kontrolü
        else if (strchr(input, ';')) {
            handle_semicolon(input);
        }
        // Pipe Kontrolü
        else if (strchr(input, '|')) {
            handle_pipe(input);
        } 
        // Tekli Komut Kontrolü
        else {
            parse_command(input, args);
            execute_command(args, 0);
        }
    }

    return 0;
}

