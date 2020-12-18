#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define MAX_DIM 100
int main(int argc, char *argv[])
{ 
	int n, i, j, fd[2], dato=1, fila, ST, SP;
	int A [MAX_DIM][MAX_DIM];
	time_t t;
	FILE *fsalida;
	n=atoi(argv[1]);
	srandom(time(&t));
	for (i=0; i<n; i++)
		for (j=0; j<n; j++)
			A[i][j]= rand() % 10;
	printf("Matriz generada (%dx%d):\n", n, n);
	for (i=0; i<n; i++)
	{ 
		for (j=0; j<n; j++) printf("%d ", A[i][j]);
			printf("\n");
	}
	SP=dup(1);
	for (i=0; i<n-1; i++)
	{ 
		pipe(fd);
		if (fork()!=0) {
			close(0);
			dup(fd[0]);
			close(fd[0]); close(fd[1]);
			read (0, &dato, sizeof(dato));

			ST=dup(1);
			close(1);
			dup(SP);
			printf("Proceso con pid %d: ", getpid());
			fila = n - i - 1;
			for (j=0; j<n; j++) printf("%d ", A[fila][j]);
			printf("\n");
			if (i!=0) write(ST,&dato,sizeof(dato));
			exit(0);
		} else {
			close(1);
			dup(fd[1]);
			close(fd[1]); close(fd[0]);
		} /* if */
	} /* for */
	close(1);
	dup(SP);
	printf("Proceso con pid %d: ", getpid());
	for (j=0; j<n; j++) printf("%d ", A[0][j]);
	printf("\n");
	if (n>1) write(1,&dato,sizeof(dato));
	exit(0);
}
