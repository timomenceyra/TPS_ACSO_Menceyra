#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char **argv)
{	
	int start, status, pid, n;
	int buffer[1];

	if (argc != 4){ printf("Uso: anillo <n> <c> <s> \n"); exit(0);}
    
	/* Parsing of arguments */
	/* TO COMPLETE */
	n = atoi(argv[1]);
	buffer[0] = atoi(argv[2]);
	start = atoi(argv[3]);
    printf("Se crearán %i procesos, se enviará el caracter %i desde proceso %i \n", n, buffer[0], start);
    
   	/* You should start programming from here... */
	int initial_pipe[2];
	int final_pipe[2];
	int pipes[n][2];

	pipe(initial_pipe);
	pipe(final_pipe);

	// Creo n pipes
	for (int i = 0; i < n; i++) {
		pipe(pipes[i]);
	}

	for (int i = 0; i < n; i++) {
		pid = fork();
		if (pid == 0) {
			if (i == start) {
				close(initial_pipe[1]);	// Cierro la escritura del pipe inicial
				read(initial_pipe[0], buffer, sizeof(int));	// Leo el valor inicial del padre
			} 
			else {
				close(pipes[(i + n - 1) % n][1]);	// Cierro la escritura del pipe del proceso anterior
				read(pipes[(i + n - 1) % n][0], buffer, sizeof(int));	// Leo el valor del proceso anterior
			}

			printf("%d recibe: %d\n", i, buffer[0]);
			buffer[0]++;	// Sumo 1 al valor que recibió el proceso
			printf("%d manda: %d\n", i, buffer[0]);

			if ((i + 1) % n == start) {
				close(final_pipe[0]);	// Cierro la lectura del pipe final
				write(final_pipe[1], buffer, sizeof(int));	// Envío el valor al pipe final
			}
			else {
				close(pipes[i][0]);	// Cierro la lectura del proceso actual
				write(pipes[i][1], buffer, sizeof(int));	// Envío el valor al proceso siguiente
			}

			exit(0);
		}

	close(initial_pipe[0]);
	write(initial_pipe[1], buffer, sizeof(int));	// Envío el valor inicial al primer proceso

	close(final_pipe[1]);	// Cierro la escritura del pipe final
	int result;
	read(final_pipe[0], &result, sizeof(int));	// Leo el resultado final del pipe

	printf("Resultado final: %d\n", result);

	for (int i = 0; i < n; i++) {
		wait(&status);	// Espero a que terminen todos los procesos hijos
	}

	return 0;
}
