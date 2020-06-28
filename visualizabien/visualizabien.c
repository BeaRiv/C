#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1

int main(int argc, char *argv[])
{
	int fd1[2], fd2[2]; /* Descriptores de ficheros para los pipes*/
	int status, pid, std_in = 0, std_out = 1, fd3;
	
	pipe(fd1);
	
	if (argc != 2){
		fprintf(stderr, "Uso: %s imagen.png.gz \n", argv[0]);
		return 1;
	}else{
		pid = fork();
		if (pid == 0){ 	/* Hijo 1, se encarga del zcat */
			close(fd1[std_in]);	/* Cerramos los descriptores de ficheros heredados del padre *
								* que no son necesarios */
			dup2(fd1[std_out], STDOUT_FILENO);
			close(fd1[std_out]);
			
			execlp("zcat", "zcat", argv[1], NULL);
			
		}else{  /* Estamos en el padre*/
			close(fd1[std_out]);
			pipe(fd2);
			pid = fork();
			
			if (pid == 0){ 	/* Hijo 2 se encarga del wc -c */
				close(fd2[std_in]);
				
				dup2(fd1[std_in], STDIN_FILENO);
				close(fd1[std_in]);
				
				dup2(fd2[std_out], STDOUT_FILENO);
				close(fd2[std_out]);
				
				execlp("/usr/bin/wc", "wc", "-c", NULL);
			}else{
				close(fd1[std_in]);
				close(fd2[std_out]);
				pid = fork();
				if (pid == 0){	/* Hijo 3 es el que se encarga del xview */
					dup2(fd2[std_in], STDIN_FILENO);
					close(fd2[std_in]);
				
					execlp("/bin/xview", "xview", "stdin", NULL);
				}
			}
		}
		close(fd2[std_in]);
		
		wait(&status); /* El padre espera por sus tres hijos */
		wait(&status);
		wait(&status);
		return 0;
	}	
}