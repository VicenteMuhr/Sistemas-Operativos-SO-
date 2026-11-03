#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "juego.h"
//argumento a hilos de fichas
typedef struct{
	PartidaKomikan *partida; //Puntero a partida Komikan
	int tipo_ficha; //tipo ficha, puma o perro
	Posicion pos_actual; //posicion actual de ficha
}ArgHiloFicha;

void iniciar_juego(PartidaKomikan *partida){ //niciar juego
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
	//PUMA en posicion inicial (como en el centro)
	partida->tablero[2][2] = PUMA;
	//PERROS posicion inicial (como en dos primeras filas)
	int perros_colocados = 0;
	for(int i = 0; i<2; i++){//perros en dos primeras filas
		for(int j=0; j<5; j++){//perro cada columna de las dos primeras filas
			if(perros_colocados<12){
				partida->tablero[i][j] = PERRO; //colocar perro
				perros_colocados++;
			}
		}
	}
}
void imprimir_tablero(PartidaKomikan *partida){
	pthread_mutex_lock(&partida->mutex_tablero);
	printf("\n Estado tablero");
	for(int i=0;i<6;i++){//iterar filas
		for(int j=0;j<5;j++){//iterar columnas
			if(partida->tablero[i][j]==VACIO) printf("."); //casillas vacias
			else if(partida->tablero[i][j]==PUMA)printf("P");//PUMA
			else if(partida->tablero[i][j]==PERRO)printf("D");//PERRO
		}
		printf("\n"); //línea nueva desp de cada fila de tablero

	}
	printf("FINAL DEL TABLERO");
	pthread_mutex_unlock(&partida->mutex_tablero);//desbloq mutex
}
int val_mov(PartidaKomikan *partida, int tipo__fecha, Posicion origen, Posicion destino){
	//verificar pos de origen y destino están e límite de tablero
	if(origen.x<0 ||origen.x>=6||origen.y<0||origen.y>=5)return 0;
	if(destino.x<0||destino.x>=6||destino.y<0||destino.y>=5) return 0;
	//verificar q casilla de destino vacia
	if(partida->tablero[destino.x][destino.y] != VACIO) return 0;
	//verificar mov válido segun reglamento
	int diff_x = abs(destino.x - origen.x);
	int diff_y = abs(destino.y - origen.y);
	//mov normal (casilla en cualquier direc.)
	if(diff_x <=1 && diff_y <=1) return 1;
	//mov captura (PUMA sobre perro)
	if(tipo_ficha ==PUMA){
		if((diff_x==2 && diff_y==0)||(diff_x==0 && diff_y==2)||(diff_x==2 && diff_y==2)){
		//^mov diagonal o recto de dos casillas
		//calc posc intermedia entre origen y destino
		int medio_x= (origen.x + destino.x)/2;
		int medio_y= (origen.y + destino.y)/2;
			//verificar si casilla media tiene PERRO
			if(partida->tablero[medio_x][medio_y] == PERRO) return 2; //mov vál. con captura
		}
	}
	return 0; //mov invalido
}
int realizar_mov(PartidaKomikan *partida, int tipo_ficha, Posicion origen, Posicion destino){
	pthread_mutex_lock(&partida->mutex_tablero); //bloq mutex acceso a tablero
	int resultado_validacion = val_mov(partida, tipo_ficha, origen, destino);//val. mov antes de exec
	if(resultado_validacion==0){//si mov invalid, desbloq mutex y devuelve 0
		pthread_mutex_unlock(&partida->mutex_tablero);
		return 0;
	}
	//exec mov actualiza tablero
	partida->tablero[destino.x][destino.y]=tipo_ficha;
	partida->tablero[origen.x][origen.y]= VACIO;
	//si mov captura, delete perro capturado, y actualizar contador capturados de perro
	if(resultado_validacion==2){
		int medio_x=(origen.x + destino.x)/2;
		int medio_y=(origen.y + destino.y)/2;
		partida->tablero[medio_x][medio_y]=VACIO;//eliminar perro de tablero
		partida->perros_capturados++;
	}
	if(partida->perros_capturados>=6) partida->juego_end=1; //6 perros capturados juego end
	pthread_mutex_unlock(&partida->mutex_tablero);//desbloq desp de mov
	return resultado_validacion;//result (1 mov normal, 2 de captura)
}

void* rutina_ficha(void *arg){ //rutina para hilos de fichas (puma y perros)
	ArgHiloFicha *datos = (ArgHiloFicha *)arg;//conertir puntero genérico en puntero tipo ArgHiloFicha
	PartidaKomikan *partida=datos->partida;//puntero de partida komikan
	//semilla para num random usando t actual e ID de hiloo
	unsigned int seed = time(NULL) ^ (unsigned long)pthread_self();
	while(1){//bucle infinito ficha siga intentando moverse hasta fin de juego)
		//verificar si juego termino antes de intentar mover
		pthread_mutex_unlock(&partida->mutex_tablero);//bloq mutex para acceder a bandera de juego terminado
		if(partida->juego_terminado){//si juego termina, terminar bucle
			pthread_mutex_unlock(&partida->mutex_tablero);
			break;
		}
		pthread_mutex_unlock(&partida->mutex_tablero);//desbloq mutex desp de verificar juego endeado
		//esperar t random 50 - 300 miliseg antes de intentar mov(simular tiempo de reaccion ficha)
		int milisegundos= 50 + (rand_r(&seed) %251);
		usleep(milisegundos*1000);//miliseg a microseg para usleep
		//generar mov random para ficha (puma/perro)
		int dir_x= (rand_r(&seed) % 3) - 1;//-1, 0, 1
		int dir_y= (rand_r(&seed) % 3) -1; // =
		if(dir_x== 0 && dir_y==0) continue;//sin mov, continue sgte bucle
		
		Posicion destino = {//calc posicion destino sumando direc al  origen
			.x = datos->pos_actual.x + dir_x,
			.y = datos->pos_actual.y + dir_y
		};
		//intentar realizar mov de ficha en partida
		int resultado = realizar_mov(partida, datos->tipo_ficha, datos->pos_actual, destino);
		if(resultado>0){//si mov existoso
			//actualizar posicion ficha en datos del hilo
			datos->pos_actual= destino;
			//imprimir tablero desp de cada mov exitoso
			imprimir_tablero(partida);
		}
	}
	free(datos);
	return NULL;
}
//func matar juego, liberar recursos
void kill_game(PartidaKomikan *partida){
	pthread_mutex_destroy(&partida->mutex_tablero);
}






