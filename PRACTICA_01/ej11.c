#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h>

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("Error, ejecución ./prog numTablas\n");
		exit(-1);
	}

	int n, i, fd[2], pid=0;
	n = atoi(argv[1]);

	printf("N = %d\n",n );

	printf("Proceso más alto: %d\n",getpid() );

	for (i = 1; i < n; i++)
	{	
		pipe(fd);
		if(fork()!=0)
		{
			close(0);
			dup(fd[0]);
			close(fd[0]); close(fd[1]);
			read(0, &pid, sizeof(pid));
			if(i!=1) write(1, &pid, sizeof(pid));
			else{
				printf("\nSoy el proceso con pid %d. Pid leído: %d\n", getpid(),pid);
				int j;
				for(j=0; j<10; j++)
				{	
					printf("%d x %d = %d\n",j,i,j*i );
				}
			}

		}else{
			close(1);
			dup(fd[1]);
			close(fd[1]); close(fd[0]);
		}	/* if */
	} /* for */

	if(n==1)
		printf("\nSoy el proceso con pid %d. Pid leído: %d\n",getpid(),pid);
	else
	{
		pid=getpid();
		write(1,&pid,sizeof(pid)); 
	}
	exit(0);
}
