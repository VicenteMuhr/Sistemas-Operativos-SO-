#include <stdio.h>
#include "planificador.h"

int main(void){

	// inicializar planificador con capacidad inicial (ej. 10)
	Planificador *plan = crear_planificador(10);
	if(plan==NULL){
		fprintf(stderr, "Error: No se pudo asignar memoria para el planificador.\n");
		return 1;
	}
	//cargar actividades desde el archivo
	printf("cargando actividades\n");
	if(cargar_planificador(plan, "plan.txt")!=0){
		fprintf(stderr, "error al cargar archivo\n");
		liberar_planificador(plan);
		return 1;
	}
	//imprimir lo interno para ver bien el parseo
	printf("\n result. parseo \n");
	for(int i=0; i<plan->total_actividades; i++){
		Actividad *act = &plan->actividades[i];
		printf("ID: %s | Nombre: %s | Tiempo: %d ms | In-degree: %d\n",
			act->id, act->nombre, act->tiempo_ms, act->in_degree);
		printf(" -> Hijos (a quienes desbloquea): ");
		if(act->num_hijos==0){
			printf("Ninguno\n");
		}else{
			for(int j=0; j<act->num_hijos; j++){
				int indice_hijo=act->hijos_indices[j];
				printf("%s ", plan->actividades[indice_hijo].id);
			}
		}
	}
	//liberar toda la memoria usada
	liberar_planificador(plan);
	printf("\n Memoria liberada, programa terminado \n");

	return 0;
}
