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

        /* Tokenizes the command string using the pipe character (|) as a delimiter using the strtok() function. 
           Each resulting token is stored in the commands[] array. 
           The strtok() function breaks the command string into tokens (substrings) separated by the pipe character |. 
           In each iteration of the while loop, strtok() returns the next token found in command. 
           The tokens are stored in the commands[] array, and command_count is incremented to keep track of the number of tokens found. */
        int command_count = 0;
        char *token = strtok(command, "|");
        while (token != NULL) 
        {
            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }

        /* You should start programming from here... */
        int prev_fd = -1;
        for (int i = 0; i < command_count; i++) 
        {
            printf("Command %d: %s\n", i, commands[i]);

            int pipe_fd[2];

            if (i < command_count - 1) 
            {
                if (pipe(pipe_fd) == -1) 
                {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }
            }
            
            pid_t pid = fork();

            if (pid < 0)
            {
                perror("fork");
                exit(EXIT_FAILURE);
            } 
            else if (pid == 0) 
            {
                // Hijo
                if (prev_fd != -1) 
                {
                    dup2(prev_fd, STDIN_FILENO);
                    close(prev_fd);
                }

                if (i < command_count - 1) 
                {
                    close(pipe_fd[0]);
                    dup2(pipe_fd[1], STDOUT_FILENO);
                    close(pipe_fd[1]);
                }

                char *argv[MAX_ARGS];
                int arg_count = 0;
                char *arg = strtok(commands[i], " ");

                while (arg != NULL && arg_count < MAX_ARGS - 1) 
                {
                    argv[arg_count++] = arg;
                    arg = strtok(NULL, " ");
                }

                argv[arg_count] = NULL;

                execvp(argv[0], argv);
                perror("execvp");
                exit(EXIT_FAILURE);
            } else {
                // Padre
                if (prev_fd != -1) {
                    close(prev_fd);
                }

                if (i < command_count - 1) {
                    close(pipe_fd[1]);
                    prev_fd = pipe_fd[0];
                }
            }
        }

        // Espera a que todos los hijos terminen
        for (int i = 0; i < command_count; i++) {
            wait(NULL);
        }
    }
    return 0;
}
