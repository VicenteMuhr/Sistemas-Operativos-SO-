#include "planificador.h"
// Genera un tiempo aleatorio entre 100 y 5000 ms
int obtener_tiempo_aleatorio(void){
	// rand() % (max-min+1)+min
	return (rand() % (5000 - 100 + 1)) + 100;
}
// Reserva la memoria inicial para el planificador
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

//Funcion aux, buscar indice de tarea en el arr usando su id
int buscar_actividad_por_id(Planificador *plan, const char *id){
	for(int i=0; i<plan->total_actividades; i++){
		if(strcmp(plan->actividades[i].id, id) == 0){
			return i;
		}
	}
	return -1; //no encontrada
}
// Lee el archivo de texto y arma el grafo DAG
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
	char *nombre_str = strtok(NULL, ":");
	char *tiempo_str = strtok(NULL, ":");
	//resto de linea (dependencias) se ignora en esta pasada

	if(id_str != NULL && nombre_str != NULL){
		Actividad *act = &plan->actividades[plan->total_actividades];

		//copiar datos a la estructura
		sscanf(id_str, " %63[^ ]", act->id); //Extrae quitando espacios
		sscanf(nombre_str, " %127[^:]", act->nombre); // hasta encontrar un : "

		int tiempo = (tiempo_str != NULL) ? atoi(tiempo_str) : 0; //convierte texto a num
		act->tiempo_ms = (tiempo>0) ? tiempo : obtener_tiempo_aleatorio();
		
		//inicializar contadores en 0
		act->in_degree=0;
		act->num_hijos=0;
		act->capacidad_hijos=0;
		act->hijos_indices=NULL;

		plan->total_actividades++;
		}
	}

//pasada 2: vincular las dependencias 
fseek(archivo, 0, SEEK_SET); //rebobinar archivo al inicio
while(fgets(linea, sizeof(linea), archivo)){
	char *id_str = strtok(linea, ":");
	if(id_str==NULL)continue;

	char id_limpio[64];
	sscanf(id_str, " %63[^ ]", id_limpio); //quien soy
	int idx_actual = buscar_actividad_por_id(plan, id_limpio);
	if(idx_actual == -1) continue;
	
	strtok(NULL, ":"); //saltar nombre
	strtok(NULL, ":"); //saltar tiempo

	char *deps_str = strtok(NULL, ":\n");//extraer sección dependencias
	if(deps_str==NULL) continue;
	//buscar corchetes
	char *inicio = strchr(deps_str, '[');
	char *fin = strchr(deps_str, ']');
	if(inicio!=NULL&&fin!=NULL&&fin>inicio){
		*fin = '\0'; //se corta texto cuando corchete cierra
		//separar dependencias por coma o espacios
		char *token_dep = strtok(inicio+1, " ,");
		while(token_dep!=NULL){
			char dep_id[64];
			if(sscanf(token_dep, "%63s", dep_id) == 1){
				int idx_padre = buscar_actividad_por_id(plan, dep_id); //de quien dependo
				if(idx_padre!=-1){
					Actividad *padre = &plan->actividades[idx_padre];
					//si padre no tiene espacio en arr, damos más memoria (realloc)
					if(padre->num_hijos>=padre->capacidad_hijos){
						padre->capacidad_hijos=(padre->capacidad_hijos==0) ? 2 : padre->capacidad_hijos * 2;
						padre->hijos_indices = realloc(padre->hijos_indices, sizeof(int)*padre->capacidad_hijos);
					}
					//padre lo anota como su hijo
					padre->hijos_indices[padre->num_hijos]= idx_actual;
					padre->num_hijos++;
					//anotar una dependencia pendiente más
					plan->actividades[idx_actual].in_degree++;
				}
			}
			token_dep=strtok(NULL, " ,"); //sgte dependencia
		}
	}
}
	fclose(archivo);
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
