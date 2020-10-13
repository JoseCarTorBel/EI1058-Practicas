#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>



int main()
{
	int estado, t1[2], t2[2];

	pipe(t1);
	if (fork()!=0)
	{ 
		pipe(t2);
		if (fork()!=0)
		{	close(t1[1]);close(t1[0]);
			pipe(t1);
			if (fork()!=0)
			{ 
				close(0); dup(t1[0]); close(t1[0]); close(t1[1]);
				close(t2[0]); close(t2[1]);
				execlp("uniq","uniq",NULL);
				perror("Error en el uniq\n");
				exit(-1);
			} else { 
				close(0); dup(t2[0]); close(t2[0]); close(t2[1]);
				close(1); dup(t1[1]); close(t1[1]); close(t1[0]);
				execlp("sort","sort",NULL);
				perror("Error en el sort\n");
				exit(-1);
			}
		} else { 
		      close(0); dup(t1[0]); close(t1[0]); close(t1[1]);
		      close(1); dup(t2[1]); close(t2[1]); close(t2[0]);
		      execlp("sed","sed","-e","1,$p",NULL);
		      perror("Error en el sed\n");
		      exit(-1);
		}
	} else { 
	    close(1); dup(t1[1]); close(t1[1]); close(t1[0]);
	    execlp("ps","ps","-l",NULL);
	    perror("Error en el ps\n");
	    exit(-1);
	}
}
