#include<stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>

int main(int argc, char *argv[])
{
	if(argc < 3){
		printf("Error.\n \t Uso ./ej fichero\n");
		exit(-1);
	}
	char dato;

	close(1);
	open(argv[1],O_WRONLY|O_CREAT|O_APPEND,0777);
	
	//write(1,&dato,sizeof(dato));
	
	execvp(argv[2],&argv[2]);
	perror("Error ejecutando el comando-> ");
	exit(-1);
}