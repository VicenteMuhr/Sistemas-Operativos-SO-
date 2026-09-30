#include "planificador.h"

int obtener_tiempo_aleatorio(void){
	// rand() % (max-min+1)+min
	return (rand() % (5000 - 100 + 1)) + 100;
}

Planificador* crear_planificador(int capacidad_inicial){
	Planificador *plan = malloc(sizeof(Planificador));
	if(plan==NULL) return NULL;

	plan->actividades=malloc(sizeof(Actividad)*capacidad_inicial);
	if(plan->actividades==NULL){
	free(plan);
	return NULL;
	}

	plan->total_actividades = 0;
	plan->capacidad = capacidad_inicial;
	return plan;
}

int cargar_planificador(Planificador *plan, const char *ruta_archivo){
	FILE *archivo = fopen(ruta_archivo, "r");
	if(archivo==NULL){
		perror("Error al abrir el archivo");
		return -1;
	}
	char linea[256];

	//pasada 1: leer id, nombre y tiempo
	while(fgets(linea, sizeof(linea), archivo)){
	//strtok corta texto cada que encuentra ":"
	char *id_str = strtok(linea, ":");
	char *nombre_str = strtrok(NULL, ":");
	char *tiempo_str = strtok(NULL, ":");
	//resto de linea (dependencias) se ignora en esta pasada

	if(id_str != NULL && nombre_str != NULL){
		Actividad *act = &plan->actividades[plan->total_activiades];

		//copiar datos a la estructura
		sscanf(id_str, " %63[^ ]", act->id); //Extrae quitando espacios
		sscanf(nombre_str, " %127[^:]", act->nombre); // hasta encontrar un : "

		int tiempo = atoi(tiempo_str); //convierte texto a num
		act->tiempo_ms = (tiemmpo>0) ? tiempo : obtener_tiempo_aleatorio();
		
		//inicializar contadores en 0
		act->in_degree=0;
		act->num_hijos=0;
		act->capacidad_hijos=0;
		act->hijos_indices=NULL;

		plan->total_actividades++;
		}
	}

//Aquí va pasada 2: rebobinar archivo (fseek), volver a leer con strtok
//buscar sección "[Dep1, Dep2]" y vincular los índices.

	fclose(archivo;
	return 0;
}

// Libera toda la memoria dinámica solicitada
void liberar_planificador(Planificador *plan){
	if (plan == NULL) return;
	for(int i=0; i<plan->total_actividades; i++){
		if(plan->actividades[i].hijos_indices != NULL){
			free(plan->actividades[i].hijos_indices);
		}
	}
	free(plan->actividades);
	free(plan);
}
