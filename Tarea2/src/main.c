#include <stdio.h>
#include <stdlib.h>
#include "servidor.h"

int main(int argc, char *argv[]){
	//verfic cant argumentos(min 3, max 4)
	if(argc<3||argc>4){
		fprintf(stderr, "Uso: %s N M [config.txt]\n", argv[0]);//print uso correcto si falla
		return EXIT_FAILURE; //salir con error
	}
	char *fin;
	//convertir arg 1 a num (N jugadores)
	long n=strtol(argv[1], &fin, 10);
	//verificar q N sea num y +
	if(*fin !='\0'||n<=0){
		fprintf(stderr,"Error: N debe ser positivo \n");
		return EXIT_FAILURE;
	}
	//arg 2 a num (M mesas)
	long m = strtol(argv[2], &fin, 10);
	//ver q M sea num y possitivo
	if(*fin!='\0'||m<=0){
		fprintf(stderr, "Error: M debe ser positivo \n");
		return EXIT_FAILURE;
	}
	//ver que cant de jugadores sea par(para poder emparejar obvio sdfjnbsdgf)
	if(n % 2 != 0){
		fprintf(stderr, "Error: N debe ser par \n");
		return EXIT_FAILURE;
	}
	//print info de torneo
	printf("------- Torneo Komikan ------- \n");
	printf("Jugadores: %ld\n", n);
	printf("Maximo mesas: %ld\n", m);
	//si hay 4 args, print tmb  el nombre del config
	if(argc==4) printf("Config: %s\n", argv[3]);
	//iniciar server con n jugadores y m mesas
	iniciar_servidor((int)n, (int)m);
	return EXIT_SUCCESS; //exitoso DEAH
} 
