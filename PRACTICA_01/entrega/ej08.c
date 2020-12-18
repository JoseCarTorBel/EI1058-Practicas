#include <stdio.h> 
#include <stdlib.h> 
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	if(argc < 4){
		printf("Error.\n \t Uso ./ej fichero_salida fichero_error comando\n");
		exit(-1);
	}
	
	int i, pid, estado;
	
	if(fork()!=0)
	{
		pid=wait(&estado);
		estado = estado & 0x0000ff00;
		estado = estado>>8;
		printf("Estado de terminación %d\n",estado);
		exit(estado);
	}else {
		close(1);
		open(argv[1],O_WRONLY|O_CREAT|O_APPEND,0700);

		close(2);
		open(argv[2],O_WRONLY|O_CREAT,0700);
	
		execvp(argv[3],&argv[3]);
		perror("Error ejecutando el comando-> ");
		exit(-1);
	}

}

