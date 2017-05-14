#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>


/*Función que calcula un número aleatorio 
*(si es entre 1 y 4 hay que pasarle 0 y 3)
*/
int calculaAleatorio (int min, int max){
	time_t segundos;
  	segundos = time(NULL);
	srand(segundos * getpid());
	return rand() % (max - min + 1) + min + 1;
	
}
//Manejador ir de misión
void mision(int sig){
	if(sig== SIGUSR2){
		printf("Soy el soldado con pid %d y me voy de misión a la colina.\n", getpid());
	}
}
//Manejador informar
void informar(int sig){ 
	if(sig==SIGUSR1){
		printf("He acabado el reconocimiento mi señor.\n");
	}else{		
		printf("Ha habido un error en el reconocimiento.\n");
	}
}

//Manejador irse de reconocimiento
void reconocimiento (int sig){
	int codigoErrorKill = 0;
	int tiempoReconocimiento = calculaAleatorio(4,14);
	//Se va a reconocer la colina
	printf("Soy %d y voy a hacer un reconocimiento.\n", getpid());
	sleep(tiempoReconocimiento);
	//Informa al cabo de que ha acabado el reconocimiento
	codigoErrorKill  = kill(getppid(), SIGUSR1);
	if (codigoErrorKill !=0) {
		 perror("Llamada a kill.");
		 exit(-1);
	}
	pause();
}

int main(int argc, char *argv[]) {
	//Declaraciones
	if(signal(SIGUSR1, reconocimiento) == SIG_ERR){
		perror("llamada a signal.\n");
		exit(-1);
	}
	if(signal(SIGUSR2, mision) == SIG_ERR){
		perror("llamada a signal.\n");
		exit(-1);
	}
	
	int i;
	int j;
	int codigoErrorSignaction = 0;
	int codigoErrorKill = 0;
	int aVerSiMuero = 0;
	int perdidas = 0;
	int noticiasSoldados = 0;
	int status;
	pid_t pidhijos;
	pid_t pidPadre = getpid();
	printf("El cabo con pid %d se ha creado.\n", pidPadre);
	int soldados[4];
	int soldadoReconocimiento = calculaAleatorio(0,3);
	
	//Comienzo
	for (i = 0; i < 4; i++) {	//Se crean los cuatro soldados
		pidhijos = fork();
		if (pidhijos == -1){
			perror("Error en la llamada a fork().\n");
		}else if (pidhijos == 0) { //Código hijos
			printf("Se ha creado el soldado %d con pid %d.\n", i+1, getpid());
			pause();
			break;
		}else{  //Código padre
			soldados[i] = pidhijos;
		}
	}
	
	if(getpid() == pidPadre){ //Código padre
		sleep(1);//Para que se termine de crear los soldados antes de mandar a un soldado de misión
		printf("Hola soy el cabo con pid %d y voy a mandar al soldado %d con pid %d a hacer un reconocimiento.\n", getpid(), soldadoReconocimiento, soldados[soldadoReconocimiento - 1]);
		if(signal(SIGUSR1, informar) == SIG_ERR){
			perror("llamada a signal.\n");
			exit(-1);
		}
		//El cabo manda a hacer un reconocimiento a un soldado
		codigoErrorKill = kill(soldados[soldadoReconocimiento - 1], SIGUSR1);
		if (codigoErrorKill !=0) {
	   		perror("Llamada a kill.");
	   		exit(-1);
		}
		pause();
		//El cabo despierta a los soldados para que se vayan de misión
		for(j; j < 4; j++){
			codigoErrorKill = kill(soldados[j], SIGUSR2);
			if (codigoErrorKill !=0) {
	   			perror("Llamada a kill.");
	   			exit(-1);
			}
		}
		while(wait(&status) > 0){ // El cabo espera a que acaben los soldados de misión.
         	 	if(WEXITSTATUS(status) == 1){
      		 		perdidas++; //Contador del cabo de los soldados muertos.
      		 	}
      		}
	}
	if(getpid() != pidPadre){ //Código hijos
		aVerSiMuero = calculaAleatorio(-1,0);
		if(aVerSiMuero == 0){
			printf("Soy el soldado con pid %d, y me han atrapado :'(\n",getpid());
			exit(1);
		}else{
			printf("Soy el soldado con pid %d, y no me han atrapado :)\n",getpid());
			exit(0);
		}
	}else{
		printf("Hemos tomado la colina y por suerte o por desgracia, hemos sufrido %d pérdidas.\n", perdidas);
	}
	return 0;
}

