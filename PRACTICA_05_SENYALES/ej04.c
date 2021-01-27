#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
int pid=0;
     
void gestor_alarma()
{  printf("Soy %d: Alarma vencida\n",getpid());
   kill(pid,SIGTSTP); }

main(int argc, char * argv[])
{ int t;
  int wait_st=-1;
  struct sigaction act;

  act.sa_flags=0;
  sigfillset(&act.sa_mask);
  act.sa_handler=gestor_alarma;
  sigaction(SIGALRM,&act,NULL);

  t=atoi(argv[1]);
  if (t <= 0) exit(-1);
  alarm (t);
  sleep(3);

  if ((pid=fork()) != 0)
  {    printf("PID padre: %d\n",getpid());
       while(wait_st < 0) {
          wait_st = wait(NULL);
          if(wait_st < 0) perror("Llamada a wait acabada con error");
       }
       printf("Finalizado wait del padre\n");
       sleep(2);
       alarm (0);
       exit(0);
  } else {
       printf("PID hijo: %d\n",getpid());
       execvp(argv[2],&argv[2]);
       exit(-1);
  }
}
