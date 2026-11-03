#ifndef JUEGO_H
#define JUEGO_H

#include <pthread.h>

//tipos de fichas
#define VACIO 0
#define PUMA 1
#define PERRO 2
//estado partida
typedef struct{
	int tablero[6][5]; //tablero de juego 6x5
	pthread_mutex_t mutex_tablero; //mutex entorno multithread
	int perros_capturados; //contador perros capturados
	int juego_end; //muestra si el juego termina
}PartidaKomikan;

//posición tablero
typedef struct{
	int x; //fila
	int y; //columna
}Posicion;

//iniciar juego, movimientos, mostrar tablero
int val_mov(PartidaKomikan *partida, int tipo_ficha, Posicion origen, Posicion destino); //valida mov legal por regla
void iniciar_juego(PartidaKomikan *partida); // inicia estado de partida y tablero
void imprimir_tablero(PartidaKomikan *partida); //imprime estado actual tablero

void kill_game(PartidaKomikan *partida);
#endif //JUEGO_H
