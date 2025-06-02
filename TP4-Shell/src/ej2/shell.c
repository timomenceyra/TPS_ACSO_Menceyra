#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200
#define MAX_ARGS 64

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

           int command_count = 0;
           char *token = strtok(command, "|");

           while (token != NULL && command_count < MAX_COMMANDS) {
               commands[command_count++] = token;
               token = strtok(NULL, "|");
           }

           int pipes[2 * (command_count - 1)];
           for (int i = 0; i < command_count - 1; i++) {
               if (pipe(pipes + i * 2) < 0) {
                   perror("pipe");
                   exit(EXIT_FAILURE);
               }
            }

            for (int i = 0; i < command_count; i++) {
                pid_t pid = fork();
                if (pid < 0) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                } else if (pid == 0) { // Proceso hijo
                    if (i > 0) { // No es el primer comando
                        dup2(pipes[(i - 1) * 2], STDIN_FILENO);
                    }

                    if (i < command_count - 1) { // No es el último comando
                        dup2(pipes[i * 2 + 1], STDOUT_FILENO);
                    }

                    // Cerrar todos los pipes en el proceso hijo
                    for (int j = 0; j < 2 * (command_count - 1); j++) {
                        close(pipes[j]);
                    }

                    // Tokenizar el comando actual
                    char *args[MAX_ARGS];
                    int arg_count = 0;
                    char *arg_token = strtok(commands[i], " ");

                    while (arg_token && arg_count < MAX_ARGS - 1) {
                        args[arg_count++] = arg_token;
                        arg_token = strtok(NULL, " ");
                    }
                    args[arg_count] = NULL;

                    execvp(args[0], args);
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }

            // Cerrar todos los pipes en el proceso padre
            for (int i = 0; i < 2 * (command_count - 1); i++) {
                close(pipes[i]);
            }

            // Esperar a que todos los procesos hijos terminen
            for (int i = 0; i < command_count; i++) {
                wait(NULL);
            }
    }
    return 0;
}
