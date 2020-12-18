#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) 
{	
	if(argc != 2)
	{
		printf("Error, ejecución ./prog numTablas\n");
		exit(-1);
	}

	int j, n, i, fd[2], pid, estado;   
	n=atoi(argv[1]);   
	
	for (i=1; i<n; i++)   
	{
		pipe(fd);     
		if (fork()==0) 
		{
			close(fd[1]);        
			close(0);         
			dup(fd[0]);         
			close(fd[0]);     
		} else {              
			close(fd[0]);                  
			if (i!=1) read (0,&pid,sizeof(pid));
			
			sleep(3);
			printf("\nProceso con pid %d. Tabla del %d\n",getpid(),i);         
			for(j=0; j<=10; j++)	printf("%d x %d = %d\n",j,i,j*i );
			
			write(fd[1],&pid,sizeof(pid));  
			close(fd[1]);

			wait(NULL);         
			exit(0);     

		}  /* if */   
	} /* for */  
	if (n>1) read (0,&pid,sizeof(pid));   
	
	printf("\nProceso con pid %d. Tabla del %d.\n",getpid(), i);
	for(j=0; j<=10; j++)	printf("%d x %d = %d\n",j,i,j*i );
	exit(0);
}
