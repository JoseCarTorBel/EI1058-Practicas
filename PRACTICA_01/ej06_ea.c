#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{ 
	int n, i, fd[2], pid=0;
	n=atoi(argv[1]);
	printf("Proceso más alto: %d\n",getpid());
	for (i=1; i<n; i++)
	{ 
		pipe(fd);
		if (fork()!=0) {
			close(0);
			dup(fd[0]);
			close(fd[0]); close(fd[1]);
			read (0,&pid,sizeof(pid));
			if (i!=1) 
				write(1,&pid,sizeof(pid));
			else
				printf("Soy el proceso con pid %d. Pid leído: %d\n", getpid(),pid);
			exit(0);
		} else {
			close(1);
			dup(fd[1]);
			close(fd[1]); close(fd[0]);
		} /* if */
	} /* for */
	
	if (n == 1){
		printf("Soy el proceso con pid %d. Pid leído: %d\n",getpid(),pid);
	}else
	{	
		pid=getpid();
		write(1,&pid,sizeof(pid));
	}
	exit(0);
}
