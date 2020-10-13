#include<stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>

int main(int argc, char *argv[])
{
	if(argc!=2){
		printf("Error. Ejecución -> ./prog codigo\n");
	}

	int i, pid, estado;
	
	if(fork()!=0) /* PADRE */ 
	{
		pid=wait(&estado);
		estado = estado & 0x0000ff00;
		estado = estado>>8;
		write(1,estado, sizeof(estado));

	}else{

	}
}