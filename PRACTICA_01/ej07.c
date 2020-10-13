#include<stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>



int main(int argc, char *argv[])
{
	if(argc!=1){
		printf("Error. Ejecución -> ./prog\n");
	}

	int pid, estado;
	
	if(fork()!=0) /* PADRE */ 
	{
		pid=wait(&estado);
		estado = estado & 0x0000ff00;
		estado = estado>>8;
		exit(estado);
	}else{
		exit(1);	
	}
}
