#include<stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>

int main(int argc, char *argv[])
{
	if(argc < 4){
		printf("Error.\n \t Uso ./ej fichero_salida fichero_error comando\n");
		exit(-1);
	}

	close(1);
	open(argv[1],O_WRONLY|O_CREAT|O_APPEND,0777);

	close(2);
	open(argv[2],O_WRONLY|O_CREAT,0777);
	
	execvp(argv[3],&argv[3]);
	perror("Error ejecutando el comando-> ");
	exit(-1);
}
