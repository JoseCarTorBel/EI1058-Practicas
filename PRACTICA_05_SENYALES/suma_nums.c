/** 
 * Acepta números enteros desde el teclado y calcule la suma de estos
 * 
 * CTRL + C -> Se muestra suma parcial acumulada, y se pone a 0 de nuevo
 * CTRL + Z -> Muestra suma global de todos los intruducidos y termina el programa con código de exito.
 * 
 * pasar 10 segundos -> "No has lleado a tiempo" + mostrar suma global calculada desde el inicio programa y aborta la 
 *                                                 devolviendo codigo error.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int pid, num;

int suma_parcial=0, suma_total=0;
int tiempo = 10;
int primera_vez_sigint = 0;

/** Se pulsa ctrl+c*/
void manejador_sigint()
{   
    primera_vez_sigint++;
    printf("\nSuma parcial desde la última interrupción: %d\n",suma_parcial);
    suma_parcial=0;
}

/** Se pulsa ctrl+z*/
void manejador_sigtstp()
{
    printf("\nSuma global: %d\n",suma_total);
    exit(EXIT_SUCCESS);    
}

void manejador_alarma()
{
    printf("\nNo has llegado a tiempo ...\n");
    printf("Suma global: %d \n",suma_total);
    exit(EXIT_FAILURE);
}


int main(int argc, char const *argv[])
{
    /* Anyadimos las señales SIGINT y SIGALRM, resto están bloqueadas*/
    sigset_t mascara;
    sigfillset(&mascara);
    sigdelset(&mascara, SIGINT);
    sigdelset(&mascara, SIGALRM);
    sigprocmask(SIG_SETMASK, &mascara, NULL);

    /* Declaración de manejadores */
    struct sigaction act;
    // SIGINT
    act.sa_handler = manejador_sigint;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGINT, &act, NULL);

    // SIGALARM
    act.sa_handler = manejador_alarma;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask); 
    sigaction(SIGALRM, &act, NULL);


    act.sa_handler = manejador_sigtstp;
    sigaction(SIGTSTP, &act, NULL);

    act.sa_handler = manejador_sigtstp;
    sigaction(SIGUSR1, &act, NULL);

    // Ignorar SIGUSR2
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigprocmask(SIG_BLOCK,&mascara,NULL);
    sigaction(SIGUSR2, &act, NULL);   

    while (1)
    {
        alarm(10);
        printf("Introduce un numero: ");
        if(scanf("%d",&num))
        {
            suma_parcial+=num;
            suma_total+=num;
        }
        else
        {
            printf("Valor introducido incorrecto\n");
        }
        num=0;

        /*Desbloqueamos sigint */
        if(primera_vez_sigint==1)
        {
            sigset_t mascara;
            sigemptyset(&mascara);
            sigaddset(&mascara, SIGTSTP);
            sigaddset(&mascara, SIGUSR1);
            sigprocmask(SIG_UNBLOCK, &mascara, NULL);

            struct sigaction act;
            act.sa_handler = manejador_sigtstp;
            act.sa_flags = 0;
            sigemptyset(&act.sa_mask);
            sigaction(SIGTSTP, &act, NULL);
            sigaction(SIGUSR1, &act, NULL);
        }
    } 
    exit(EXIT_SUCCESS);
}
