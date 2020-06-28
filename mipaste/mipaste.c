#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1

int main(int argc, char *argv[])
{
	int proc1[2], proc2[2];
	int std_in = 0, std_out = 1;
	int pid, status, leidos, fich;
	char buffer[1024];
	
	
	if (argc != 4){
		fprintf (stderr, "Uso: %s fichero1 ficher2 fichero3", argv[0]);
		return 1;
	}else{
		pipe(proc1);
		pid = fork();
		if (pid == 0){ 
		/* Primer hijo, realiza el paste */
			close(proc1[std_in]); /* Cerramos el descriptor de fichero no necesario */
			dup2(proc1[std_out], STDOUT_FILENO);
			close(proc1[std_out]);
			
			execlp("paste", "paste", argv[1], argv[2], NULL);
			
		}else{
			/* Estamos en el padre, cerramos el descriptor de fichero no necesario *
			 * para que el siguiente hijo lo herede cerrado ya */
			close(proc1[std_out]);
			
			pipe(proc2);
			pid = fork();
			
			if (pid == 0){
			/* Segundo hijo, realiza el sort */
				close(proc2[std_in]); /* Cerramos el descriptor de fichero abierto y no *
									   * necesario */
				
				dup2(proc1[std_in], STDIN_FILENO);
				close(proc1[std_in]);
				dup2(proc2[std_out], STDOUT_FILENO);
				close(proc2[std_out]);
				
				execlp("sort", "sort", argv[3], NULL);
				
			}else{
				/* Estamos en el padre, se encarga de redireccionar la salida estándar *
				* al fichero*/
				
				/* Cerramos los descriptores de ficheros no necesarios */
				close(proc1[std_in]);
				close(proc2[std_out]);
				
				dup2(proc2[std_in], STDIN_FILENO);
				close(proc2[std_in]);
				
				/* fich será el descriptor de fichero donde tenemos que redireccionar la *
				 * salida*/
				fich = open(argv[3], O_RDWR|O_CREAT|O_TRUNC, S_IRWXU);
				dup2(fich, STDOUT_FILENO);
				
				do{
					leidos = read(STDIN_FILENO, buffer, sizeof(buffer));
					leidos = write(fich, buffer, leidos);
				}while(sizeof(buffer) == leidos);
				
				/* Esperamos por cada hijo */
				wait(&status);
				wait(&status);
			}
		}
		return 0;
	}
}