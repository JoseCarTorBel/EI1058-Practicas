/* @author Jose Carlos Torró Belda */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>


#define PROMPT    "\n=> "
#define ESPACIO   ' '

#define MAX_BUF   1024
#define MAX_ARGS  32
#define MAX_COMANDOS    10

#define HOST 1024
#define USER 64
#define PATH 256


//#define DEBUG

struct comando{
  char *argv[MAX_ARGS];
  int nargs;
};

int status;
char aux[32];

const char *getUserName();
void ejecuta_comando_simple(struct comando *comando);
void redirect_entry(struct comando *comando);
void redirect_output(struct comando *comando);
void ejecuta_procesos_tuberias(int num_comandos, struct comando lista_comandos[]);
int arrange(char *buffer);
int makeargs(char *buffer, char *args[]);
int desglosar_tub(char *buffer, struct comando lista_comandos[]);
int contiene_simbolo(char *cadena, char simbolo);
void almacena_estado(int estado);




const char *getUserName()
{
 	uid_t uid = geteuid();
  	struct passwd *pw = getpwuid(uid);
  	if (pw)
  	{
  		return pw->pw_name;
  	}
  	return "";
}


void redirect_entry(struct comando *comando)
{
	/* Redirección entrada */ 
	int i;
	for(i=1; i< comando -> nargs; i++)
	{	
		/* strcmp():
				0  -> equals
				<0 -> less than the second
				>0 -> greater than second
		*/
		if(strcmp(comando-> argv[i],"<")==0)
		{
			int entrada_standar = open(comando->argv[i+1], O_RDONLY);
			close(0); dup(entrada_standar); close(entrada_standar);
			/* Ponemos a null estos valores para que execvp no lo los ejecute */
			comando -> argv[i] =NULL;
			comando -> argv[i+1] = NULL;

			break;
		}
	}
}


void redirect_output(struct comando *comando) 
{
	int i;

	for (i=1; i< comando -> nargs ; i++ )
	{
		/* 
		Posibilidades salida standar
			>	Salida standar añadir/ machacar fichero
			>>	Salida estandar añadir a final fichero
			2>	Redirección error
			2>>	Redirección error añadir al final fichero
		*/
		if(comando -> argv[i] != NULL)
		{	int salida_estandar;
			if(strcmp(comando-> argv[i],">")==0) /* >	Salida standar añadir/ machacar fichero */
			{
				salida_estandar = open(comando -> argv[i+1], O_CREAT | O_TRUNC | O_WRONLY, 0644);
				close(1); dup(salida_estandar); close(salida_estandar);
				comando -> argv[i] = NULL;
				comando -> argv[i+1] = NULL;
				i++;
			}
			else if(strcmp(comando-> argv[i],">>") == 0) /* >>	Salida estandar añadir a final fichero */
			{
				salida_estandar = open(comando -> argv[i+1], O_CREAT | O_APPEND | O_WRONLY, 0644);
				close(1); dup(salida_estandar); close(salida_estandar);
				comando -> argv[i] = NULL;
				comando -> argv[i+1] = NULL;
				i++;
			}
			else if(strcmp(comando-> argv[i],"2>") == 0) /* 2>	Redirección error */
			{
				salida_estandar = open(comando -> argv[i+1], O_CREAT | O_TRUNC | O_WRONLY, 0644);
				close(2); dup(salida_estandar); close(salida_estandar);
				comando -> argv[i] = NULL;
				comando -> argv[i+1] = NULL;
				i++;
			}
			else if(strcmp(comando-> argv[i],"2>>")==0) /* 2>>	Redirección error añadir al final fichero */
			{
				salida_estandar = open(comando -> argv[i+1], O_CREAT | O_APPEND | O_WRONLY, 0644);
				close(2); dup(salida_estandar); close(salida_estandar);
				comando -> argv[i] = NULL;
				comando -> argv[i+1] = NULL;
				i++;
			}
		}
	}
}


void almacena_estado(int estado)
{
	estado = estado &0x0000ff00;
	estado=estado >> 8;
	sprintf(aux, "%d",estado);
	setenv("?",aux,1);
}


/* Ejecuta un comando simple sin tuberías*/
void ejecuta_comando_simple(struct comando *comando)
{

	if(fork()==0)
	{	
		/* Comprobar si se redirecciona la entrada standar */
		redirect_entry(comando);
		redirect_output(comando);
		
		#ifdef DEBUG
		int i;
		for(i=0; i< comando-> nargs; i++){
			printf("%s\n",comando -> argv[i] );
		}
		#endif
		
		execvp(comando -> argv[0], comando -> argv);
		perror("Error ejecutando comando simple ");
		exit(-1);
	} else {
		wait(&status);
		almacena_estado(status);
	}
}


void ejecuta_procesos_tuberias(int num_comandos, struct comando lista_comandos[])
{
	int i, fd[2];

	/* El proceso principal se debe esperar */
	if(fork()==0)
	{	
		for(i=0; i<num_comandos-1; i++)
		{
			pipe(fd);
			if(fork() == 0)/* Hijo */
		   	{
		    	close(1); dup(fd[1]); close(fd[1]); close(fd[0]);	// Escribir en la tubería

		   	} else { /* Padre */
		    	close(0); dup(fd[0]); close(fd[0]); close(fd[1]);	// Redirección entrada
		      	/* Último comando a ejecutar*/
		      	
		      	#ifdef DEBUG	
		    		printf("i-> %d \t Comando -> %s\n",i,lista_comandos[num_comandos-1-i].argv[0] );
		      	#endif

		      	if(num_comandos-1-i == num_comandos-1)
		      	{	
		      		/* Redirección salida standar en caso de ser necesario */
		      		redirect_output(&lista_comandos[num_comandos-1-i]);
		      	}
		      	
		      	execvp(lista_comandos[num_comandos-1-i].argv[0],lista_comandos[num_comandos-1-i].argv);
		      	perror("Error en la ejecución");
		      	exit(-1);
		   } /* end fork */
		} 
		printf("Comando -> %s\n",lista_comandos[num_comandos-1-i].argv[0] );
      	
		redirect_entry(&lista_comandos[num_comandos-1-i]);
		execvp(lista_comandos[num_comandos-1-i].argv[0],lista_comandos[num_comandos-1-i].argv);
		perror("Error en la ejecución");
		exit(-1);
	}else {
		wait(&status);
		almacena_estado(status);
	}		
}


int arrange(char *buffer) 
{
  	int i = 0, j = 0;
	
	while (buffer[i]) {
        while (buffer[i] == ESPACIO) i++;

        while ((buffer[i] != ESPACIO) && buffer[i]) 
            buffer[j++] = buffer[i++];
        
        if (! buffer[i]) {
            if(buffer[i-2] == ESPACIO) {
                buffer[j-2] = '\0';
            } else {
                buffer[j-1] = '\0';
            }
        } else {
            buffer[j++] = ESPACIO;
        }
    }
	
 return(j);
}


int makeargs(char *buffer, char *args[])
{
	int i=0;
	char *puntero;

	puntero = buffer;
	args[i++]=puntero;

	while(puntero = strchr(puntero, ESPACIO))
	{
		*puntero++ = '\0';
		args[i++] = puntero;
	}
	args[i] = NULL;
	return (i);
}


int desglosar_tub(char *buffer, struct comando lista_comandos[])
{
	char *p1, *p2;
  	int i,j,n;
  	int ncomandos=0;

  	p1=buffer;
  	i=0;


	#ifdef DEBUG
		printf("\nLínea de comandos: #%s#\n", buffer);
	#endif

	n=arrange(p1);

	if(n==1){		
		return 0; /* No hay comandos, sino blancos o <CR> */		
	}

	 while (p2=strchr(p1,'|')) {
        if (*(p2-1)==ESPACIO) {
            *(p2-1)='\0'; 
        } else {
            *p2='\0';
        }
       
        p2++;
        lista_comandos[ncomandos].nargs = makeargs(p1,lista_comandos[ncomandos].argv);
        ncomandos++;
        if (p2!=NULL) { 
            if (*p2!=ESPACIO) 
                p1=p2; 
            else 
                p1=p2+1;
        }
    } /* WHILE */

	lista_comandos[ncomandos].nargs = makeargs(p1,lista_comandos[ncomandos].argv);
    ncomandos++;

	#ifdef DEBUG
		for (i=0; i<ncomandos; i++)
	  	{  
	  		printf("Comando %d (%d argumentos):\n",i,lista_comandos[i].nargs);
	     	j=0;
	     	while(lista_comandos[i].argv[j])
	     	{ 
	     		printf("comando[%d].argv[%d] (dir. %016lX) = #%s#\n", i, j,(unsigned long) (lista_comandos[i].argv[j]), lista_comandos[i].argv[j]);
	       		j++;
	     	}
	  	}
	#endif	
	
	/* Utilización variables de entorno -> setenv*/
	/* Buscamos la variable de entorno*/
	  	/*	TODO -> ACABAR*/
	for(i=0; i< ncomandos; i++)
	{
		j=0;
		while(lista_comandos[i].argv[j])
		{
			if(contiene_simbolo(lista_comandos[i].argv[j],'$'))
			{
				char *variable_entorno = getenv(lista_comandos[i].argv[j]+1);
				//printf("Variable -> %s\n", variable_entorno);
				lista_comandos[i].argv[j] = variable_entorno;
			}
			j++;
		}
	}

	 /* Número de componentes del vector lista_comandos (empezando en 1) */
	return ncomandos;
}

int contiene_simbolo(char *cadena, char simbolo)
{
	int i=0;
	while(cadena[i])
	{
		if(cadena[i]==simbolo)
		{
			return 1;
		}
		i++;
	}
	return 0;
}



/* ___________________________________________________ */

int main(int argc, char const *argv[])
{

	#ifdef DEBUG
		int i;
	#endif
	  
	int estado_anterior;

    char buffer[MAX_BUF], *args[MAX_ARGS];
    struct comando lista_comandos[MAX_COMANDOS];

    puts("\n MINISHELL \n");

   // char user = getUserName();

    char hostname[HOST];
	char path[PATH];
	char user[USER];

	hostname[HOST] = '\0';
	gethostname(hostname, HOST);

    getlogin_r(user, sizeof(user));


    /* Bucle infinito */
    while(1) 
    {
        getcwd(path, sizeof(path));

    	printf("%s@%s:%s$ ",user,hostname,path);
    //	printf("%s", PROMPT);
    	if(fgets(buffer, MAX_BUF, stdin)==NULL){
    		continue;
    	}

    	#ifdef DEBUG
    		printf("Estoy aquí 01\n");
    	#endif

    	int num_comandos = desglosar_tub(buffer, lista_comandos);
    	

    	#ifdef DEBUG
    		printf("Estoy aquí 02\n");
    	#endif

    	if(!num_comandos) {
       		continue;
    	}
    	
		#ifdef DEBUG
    		printf("Num_comandos = %d\n", num_comandos);
    	#endif
    	/* TODO Redireccionar entrada estandar */
    	if(num_comandos == 1) /* En caso de ser comando simple */
    	{	
    		if(strcmp(lista_comandos[0].argv[0],"exit")==0)
    		{	
    			printf("\nEXIT MINISHELL \n");
    			exit(1);
    		}else if(strcmp(lista_comandos[0].argv[0],"cd")==0)
    		{	
    			if(lista_comandos[0].argv[1] == NULL)
    			{	/*cd*/
    				char newPath[PATH] = "/home/";
    				strcat(newPath,user);
    				chdir(newPath);
    			}else{
    				chdir(lista_comandos[0].argv[1]);
    			}
    		}else if(contiene_simbolo(lista_comandos[0].argv[0], '='))
    		{	

    			char *variables = strtok(lista_comandos[0].argv[0],"=");
    			char *var = variables;
    			variables = strtok(NULL,"=");
    			char *assign_var = variables;
    			//printf("%s\n", assign_var);
    			setenv(var, assign_var, 1);	/* Asigna a variable de entorno. Con el 1 la sobreescribe si existe */
    		}else{
    			ejecuta_comando_simple(&lista_comandos[0]);
    		}
    	}else {
    		ejecuta_procesos_tuberias(num_comandos, lista_comandos);
    	}
    	printf("\n");
    } /* while(1) */
} /* Main */














