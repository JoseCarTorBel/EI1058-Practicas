#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int pid;

void gestor_int()
{
    printf("Soy %d: Recibida SIGINT.\n", getpid());
}

void gestor_alarma()
{
 printf("Soy %d: Recibida SIGALRM.\n", getpid());
 if (kill(pid, SIGKILL) < 0) {
    perror("Error enviando SIGKILL");
    exit(EXIT_FAILURE);
 }
}

main(int argc, char *argv[])
{
 struct sigaction act;
 sigset_t set;
 unsigned int p1, p2;

 if (argc != 3) {
    printf ("Número de parámetros (%d) incorrecto.\nSe requieren exactamente 2.\n", argc - 1);
    exit(EXIT_SUCCESS);
 }
 p1 = atoi(argv[1]);
 p2 = atoi(argv[2]);

 if (sigfillset(&act.sa_mask) < 0) {
    perror("Error creando conjunto lleno 1");
    exit(EXIT_FAILURE);
 }
 act.sa_flags = 0;
 act.sa_handler = gestor_int;
 if (sigaction(SIGINT, &act, NULL) < 0) {
    perror("Error armando SIGINT");
    exit(EXIT_FAILURE);
 }
 act.sa_handler = gestor_alarma;
 if (sigaction(SIGALRM, &act, NULL) < 0) {
    perror("Error armando SIGALRM");
    exit(EXIT_FAILURE);
 }
 
 if (sigfillset(&set) < 0) {
    perror("Error creando conjunto lleno 2");
    exit(EXIT_FAILURE);
 }
 if (sigdelset(&set, SIGINT) < 0) {
    perror("Error eliminando SIGINT del conjunto");
    exit(EXIT_FAILURE);
 }
 if (sigdelset(&set, SIGALRM) < 0) {
    perror("Error eliminando SIGALRM del conjunto");
    exit(EXIT_FAILURE);
 }
 if (sigprocmask(SIG_SETMASK, &set, NULL) < 0) {
    perror("Error escribiendo bloqueadas");
    exit(EXIT_FAILURE);
 }
 if (sigaddset(&set, SIGINT) < 0) {
    perror("Error añadiendo SIGINT al conjunto");
    exit(EXIT_FAILURE);
 }

 alarm(10);

 if ((pid = fork()) < 0) { 
    perror("Error en la llamada fork().");
    exit(EXIT_FAILURE);
 }

 if (pid != 0) {
    printf("PID padre: %d.\n", getpid());
    sleep(p1);
    if (sigprocmask(SIG_BLOCK, &set, NULL) < 0) {
       perror("Error añadiendo bloqueadas");
       exit(EXIT_FAILURE);
    }
    pause();
    printf("Fin padre.\n");
 }
 else {
    printf("PID hijo: %d.\n", getpid());
    sleep(p2);
    execlp("./programa1", "programa1", NULL);
       perror("Error en execlp");
       exit(EXIT_FAILURE);
 }
}
