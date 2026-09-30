#include "planificador.h"

int obtener_tiempo_aleatorio(void){
	// rand() % (max-min+1)+min
	return (rand() % (5000 - 100 + 1)) + 100;
}
