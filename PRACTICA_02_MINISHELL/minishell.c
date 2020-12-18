#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>



#define PROMPT    "\n=> "
#define ESPACIO   ' '

#define MAX_BUF   1024
#define MAX_ARGS  32
#define MAX_COMANDOS    10

struct comando{
  char *argv[MAX_ARGS];
  int nargs;
};


#define DEBUG

int arrange(char *buffer) 
{

  int i = 0, j = 0;

  while(buffer[i]) {
    while(buffer[i] == ESPACIO) i++;
    while((buffer[i] != ESPACIO) && buffer[i])
          buffer[j++] = buffer[i++];
    if(! buffer[i]) {
       if(buffer[i-2] == ESPACIO)
          buffer[j-2] = '\0';
       else buffer[j-1] = '\0';
    }
    else
       buffer[j++] = ESPACIO;
  }
 return(j);
}

int makeargs(char *buffer, char *args[]) 
{

  int i = 0;
  char *puntero;

  puntero = buffer;
  args[i++] = puntero;

  while(puntero = strchr(puntero, ESPACIO)) {
    *puntero++ = '\0';
    args[i++] = puntero;
  }
  args[i] = NULL;
  return(i);
}

int desglosar_tub (char *buffer, struct comando lista_comandos[]) 
{
/* Desglosa una secuencia de comandos enlazados mediante tuber�as 
   (y almacenada a partir de la posici�n "buffer") en los diferentes
   comandos simples de esta, alacenando cada uno de ellos en una estructura 
   de tipo "argc-argv" del vector "lista_comandos"  */

  char *p1, *p2;
  int i,j,n;
  int ncomandos=0;

  p1=buffer;
  i=0;

  #ifdef DEBUG
    printf("\nL�nea de comandos: #%s#\n", buffer);
  #endif

  n=arrange(p1);
  if (n==1) return(0); /* No hay comandos, sino blancos o <CR> */
  while (p2=strchr(p1,'|'))
  {  
    if (*(p2-1)==ESPACIO) *(p2-1)='\0'; 
    else *p2='\0';
    
    p2++;
    lista_comandos[ncomandos].nargs = makeargs(p1,lista_comandos[ncomandos].argv);
    
    ncomandos++;
    if (p2!=NULL) 
    { 
      if (*p2!=ESPACIO) p1=p2; 
      else p1=p2+1; 
    }
  } /* fin while */

  lista_comandos[ncomandos].nargs = makeargs(p1,lista_comandos[ncomandos].argv);

  ncomandos++; 

  #ifdef DEBUG
    for (i=0; i<ncomandos; i++)
    {  
      printf("Comando %d (%d argumentos):\n",i,lista_comandos[i].nargs);
      j=0;
      while(lista_comandos[i].argv[j])
      { printf("comando[%d].argv[%d] (dir. %016lX) = #%s#\n", i, j,(unsigned long) (lista_comandos[i].argv[j]), lista_comandos[i].argv[j]);
        j++;
      }
    }
  #endif
  return(ncomandos);  /* N�mero de componentes del vector lista_comandos 
                         (empezando a contar desde 1) */
}


int main( int argc, char *argv[] ) 
{

   #ifdef DEBUG
      int i;
   #endif

   char buffer[MAX_BUF], *args[MAX_ARGS];
   struct comando lista_comandos[MAX_COMANDOS];

   puts("\nEjemplo sencillo de Minishell\n");

   while(1) {

      printf(PROMPT);
      if (fgets(buffer,  MAX_BUF, stdin) == NULL) 
         continue;
      arrange(buffer);
      
      #ifdef DEBUG
          printf("Entrada: #%s#\n", buffer);
          i = 0;
      #endif
      
      makeargs(buffer, args);
      
      #ifdef DEBUG
        while(args[i]) 
        {
            printf("\nargs[%d] (dir. %016lX) = #%s#", i, (unsigned long) (args[i]), args[i]);
            i++;
        }
     #endif
         
      /* Resolución del seminario */
      int fd[2], i;
      
      int n_comandos = desglosar_tub(buffer,lista_comandos);
      if(fork()==0)
      {
        for(i = 0; i<n_comandos; i++)
        {
           pipe(fd);
           if(fork() == 0)
           {
              close(1); dup(fd[1]); close(fd[1]); close(fd[0]);
           } else {
              close(0); dup(fd[0]); close(fd[0]); close(fd[1]);
              execvp(lista_comandos[n_comandos-1-i].argv[0],lista_comandos[n_comandos-1-i].argv);
              perror("Error en la ejecución");
              exit(-1);
           } /* fork */
        }
      }else{
        wait(NULL);
      }
   }/* while */
}
