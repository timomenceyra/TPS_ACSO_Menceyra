#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200
#define MAX_ARGS 64

int contar_argumentos(const char *comando) {
    // Cuenta los argumentos en un comando, considerando comillas
    int count = 0;
    int in_quotes = 0;

    for (const char *p = comando; *p; ++p) {
        if (*p == '"') {
            in_quotes = !in_quotes;
        } else if (!in_quotes && isspace(*p)) {
            while (isspace(*p)) p++;
            count++;
        }
    }
    return count + 1;
}

int separar_comandos(char *command, char *commands[]) {
    // Separa los comandos por pipe (|) y los almacena en commands
    int command_count = 0;
    int in_quotes = 0;
    char *start = command;

    for (char *p = command; ; ++p) {
        if (*p == '"') in_quotes = !in_quotes;

        // Pipe afuera de comillas o fin de línea
        if ((*p == '|' && !in_quotes) || *p == '\0') {
            size_t len = p - start;

            while (len > 0 && isspace(start[0])) { start++; len--; } // Elimina espacios a izquierda
            while (len > 0 && isspace(start[len - 1])) len--; // Elimina espacios a derecha

            if (len == 0) {
                // Si el comando está vacío, error
                fprintf(stderr, "Error: comando vacío\n");
                for (int i = 0; i < command_count; i++) {
                    free(commands[i]);
                }
                return -1;
            }

            commands[command_count] = malloc(len + 1);
            if (!commands[command_count]) {
                perror("malloc");
                exit(1);
            }

            strncpy(commands[command_count], start, len);
            commands[command_count][len] = '\0';

            // Verifica si se excede el número máximo de argumentos
            if (contar_argumentos(commands[command_count]) > MAX_ARGS) {
                fprintf(stderr, "Error: se excedió la cantidad máxima de argumentos permitidos\n");
                for (int i = 0; i <= command_count; i++) {
                    free(commands[i]);
                }
                return -1;
            }

            command_count++;

            if (*p == '\0') break; // Fin de línea

            start = p + 1; // Avanza al siguiente comando
        }
    }
    return command_count;
}

int main() {
    char command[256];
    char *commands[MAX_COMMANDS];

    while (1) 
    {
        if (isatty(STDIN_FILENO)) {
            printf("Shell> ");
            fflush(stdout);
        }
        
        /*Reads a line of input from the user from the standard input (stdin) and stores it in the variable command */
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;  // Si llegó EOF
        }
        
        /* Removes the newline character (\n) from the end of the string stored in command, if present. 
           This is done by replacing the newline character with the null character ('\0').
           The strcspn() function returns the length of the initial segment of command that consists of 
           characters not in the string specified in the second argument ("\n" in this case). */
        command[strcspn(command, "\n")] = '\0';

        // Terminar la shell si el usuario escribe "exit"
        if (strcmp(command, "exit") == 0) {
            break;
        }

        /* Tokenizes the command string using the pipe character (|) as a delimiter using the strtok() function. 
           Each resulting token is stored in the commands[] array. 
           The strtok() function breaks the command string into tokens (substrings) separated by the pipe character |. 
           In each iteration of the while loop, strtok() returns the next token found in command. 
           The tokens are stored in the commands[] array, and command_count is incremented to keep track of the number of tokens found. */

        // Separa los comandos por pipe
        int command_count = separar_comandos(command, commands);
        if (command_count < 0) continue;

        int pipes[MAX_COMMANDS][2];
        pid_t pids[MAX_COMMANDS];

        // Crea un proceso hijo por cada comando
        for (int i = 0; i < command_count; i++) {
            // Crea un pipe para cada comando excepto el último
            if (i < command_count - 1 && pipe(pipes[i]) < 0) {
                perror("pipe");
                exit(1);
            }

            pids[i] = fork();
            if (pids[i] == 0) {
                if (i > 0) {
                    dup2(pipes[i - 1][0], STDIN_FILENO);
                }

                if (i < command_count - 1) {
                    dup2(pipes[i][1], STDOUT_FILENO);
                }

                // Cierra todos los pipes en el proceso hijo
                for (int j = 0; j < command_count - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }

                // Ejecuta el comando
                execlp("sh", "sh", "-c", commands[i], (char *)NULL);
                perror("exec");
                exit(1);
            } else if (pids[i] < 0) {
                perror("fork");
                exit(1);
            }
        }

        // Cierra los pipes en el proceso padre
        for (int i = 0; i < command_count - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        // Espera a que todos los procesos hijos terminen
        for (int i = 0; i < command_count; i++) {
            waitpid(pids[i], NULL, 0);
        }

        // Libera la memoria de los comandos
        for (int i = 0; i < command_count; i++) {
            free(commands[i]);
        }
    }
    return 0;
}
