#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "juego.h"

extern pthread_mutex_t mutex_consola;

void iniciar_juego(PartidaKomikan *partida){ //iniciar juego
	//iniciar mutex tablero
	pthread_mutex_init(&partida->mutex_tablero, NULL);
	partida->perros_capturados = 0;
	partida->juego_end = 0;

	//limpiar tablero VACIO (0)
	for(int i=0; i<6;i++){
		for(int j=0;j<5;j++){
			partida->tablero[i][j] = VACIO;
		}
	}
	//PUMA en posicion inicial
	partida->tablero[2][2] = PUMA;
	//PERROS posicion inicial
	int perros_colocados = 0;
	for(int i = 0; i<2; i++){
		for(int j=0; j<5; j++){
			if(perros_colocados<12){
				partida->tablero[i][j] = PERRO;
				perros_colocados++;
			}
		}
	}
}

void imprimir_tablero(PartidaKomikan *partida){
	char copia[6][5];

	//copiar estado rapido bajo el mutex del tablero
	pthread_mutex_lock(&partida->mutex_tablero);
	for(int i=0; i<6; i++){
		for(int j=0; j<5; j++){
			copia[i][j] = partida->tablero[i][j];
		}
	}
	pthread_mutex_unlock(&partida->mutex_tablero);

	//imprimir en pantalla usando solo el mutex de consola
	pthread_mutex_lock(&mutex_consola);
	printf("\n Estado tablero\n");
	for(int i=0; i<6; i++){
		for(int j=0; j<5; j++){
			if(copia[i][j]==VACIO) printf(".");
			else if(copia[i][j]==PUMA) printf("P");
			else if(copia[i][j]==PERRO) printf("D");
		}
		printf("\n");
	}
	printf("FINAL DEL TABLERO\n");
	pthread_mutex_unlock(&mutex_consola);
}

int val_mov(PartidaKomikan *partida, int tipo_ficha, Posicion origen, Posicion destino){
	if(origen.x<0 ||origen.x>=6||origen.y<0||origen.y>=5)return 0;
	if(destino.x<0||destino.x>=6||destino.y<0||destino.y>=5) return 0;
	if(partida->tablero[destino.x][destino.y] != VACIO) return 0;

	int diff_x = abs(destino.x - origen.x);
	int diff_y = abs(destino.y - origen.y);

	if(diff_x <=1 && diff_y <=1) return 1;

	if(tipo_ficha == PUMA){
		if((diff_x==2 && diff_y==0)||(diff_x==0 && diff_y==2)||(diff_x==2 && diff_y==2)){
			int medio_x= (origen.x + destino.x)/2;
			int medio_y= (origen.y + destino.y)/2;
			if(partida->tablero[medio_x][medio_y] == PERRO) return 2;
		}
	}
	return 0;
}

int realizar_mov(PartidaKomikan *partida, int tipo_ficha, Posicion origen, Posicion destino){
	pthread_mutex_lock(&partida->mutex_tablero);
	int resultado_validacion = val_mov(partida, tipo_ficha, origen, destino);
	if(resultado_validacion==0){
		pthread_mutex_unlock(&partida->mutex_tablero);
		return 0;
	}

	partida->tablero[destino.x][destino.y]=tipo_ficha;
	partida->tablero[origen.x][origen.y]= VACIO;

	if(resultado_validacion==2){
		int medio_x=(origen.x + destino.x)/2;
		int medio_y=(origen.y + destino.y)/2;
		partida->tablero[medio_x][medio_y]=VACIO;
		partida->perros_capturados++;
		if(partida->perros_capturados>=6){
			partida->juego_end = 1;
		}
	}
	if(!partida->juego_end) {
		Posicion pos_puma = {-1, -1};
		for(int i = 0; i < 6; i++) {
			for(int j = 0; j < 5; j++) {
				if(partida->tablero[i][j] == PUMA) {
					pos_puma = (Posicion){i, j};
					break;
				}
			}
		}
		if(pos_puma.x != -1 && puma_acorralado(partida, pos_puma)) {
			pthread_mutex_lock(&mutex_consola);
			printf("\nPerros acorralan al puma\n");
			pthread_mutex_unlock(&mutex_consola);
			partida->juego_end = 1;
		}
	}
	pthread_mutex_unlock(&partida->mutex_tablero);
	return resultado_validacion;
}

int puma_acorralado(PartidaKomikan *partida, Posicion pos_puma) {
	int dir_x[] = {-1, -1, -1, 0, 0, 1, 1, 1};
	int dir_y[] = {-1, 0, 1, -1, 1, -1, 0, 1};

	for(int i = 0; i < 8; i++) {
		Posicion dest1 = {pos_puma.x + dir_x[i], pos_puma.y + dir_y[i]};
		if(val_mov(partida, PUMA, pos_puma, dest1) > 0) {
			return 0; 
		}
		
		Posicion dest2 = {pos_puma.x + (dir_x[i] * 2), pos_puma.y + (dir_y[i] * 2)};
		if(val_mov(partida, PUMA, pos_puma, dest2) > 0) {
			return 0; 
		}
	}
	return 1;
}

void* rutina_ficha(void *arg) {
	ArgHiloFicha *datos = (ArgHiloFicha *)arg;
	PartidaKomikan *partida = datos->partida;
	unsigned int seed = time(NULL) ^ (unsigned long)pthread_self();

	while(1) {
		pthread_mutex_lock(&partida->mutex_tablero);
		if(partida->juego_end) {
			pthread_mutex_unlock(&partida->mutex_tablero);
			break;
		}
		pthread_mutex_unlock(&partida->mutex_tablero);

		int milisegundos = 50 + (rand_r(&seed) % 251);
		usleep(milisegundos * 1000);

		int dir_x = (rand_r(&seed) % 3) - 1;
		int dir_y = (rand_r(&seed) % 3) - 1;
		if(dir_x == 0 && dir_y == 0) continue;
		
		int distancia = 1;
		if (datos->tipo_ficha == PUMA && (rand_r(&seed) % 2 == 0)) {
			distancia = 2;
		}

		Posicion destino = {
			.x = datos->pos_actual.x + (dir_x * distancia),
			.y = datos->pos_actual.y + (dir_y * distancia)
		};

		int resultado = realizar_mov(partida, datos->tipo_ficha, datos->pos_actual, destino);
		if(resultado > 0) {
			datos->pos_actual = destino;
			pthread_mutex_lock(&mutex_consola);
			printf("[%s] se movio a [%d, %d]\n", 
				(datos->tipo_ficha == PUMA) ? "Puma" : "Perro", 
				destino.x, destino.y);
			pthread_mutex_unlock(&mutex_consola);
		}
	}
	free(datos);
	return NULL;
}

void kill_game(PartidaKomikan *partida){
	pthread_mutex_destroy(&partida->mutex_tablero);
}
