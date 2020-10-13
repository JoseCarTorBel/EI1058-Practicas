#include<stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>

int main(int argc, char *argv[])
{
	if(argc != 3){
		printf("Error.\n \t Uso ./ej fichero\n");
		exit(-1);
	}
	char dato;

	close(1);
	open(argv[1],O_WRONLY|O_CREAT|O_APPEND,0777);
		
	execlp("ls","ls","-l",argv[2],NULL);
	perror("Error ejecucanto ls");
	exit(-1);
}
