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
	if(argc!=2){
		printf("Error. Ejecución -> ./prog codigo\n");
	}

	int i, pid, estado;
	
	if(fork()!=0) /* PADRE */ 
	{
		pid=wait(&estado);
		estado = estado & 0x0000ff00;
		estado = estado>>8;
		printf("Hola soy el padre y mi hijo me ha dado esto %d\n",estado);
	}else{
		printf("Hola soy el hijo de mi padre y mi código es %d\n",atoi(argv[1]) );
		exit(atoi(argv[1]));	
	}
}
