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
		
		act->pid = -1;
		act->estado=ESTADO_PENDIENTE;
		act->pipe_fd[0]=-1;
		act->pipe_fd[1]=-1;
		act->num_dependencias=0;		

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

//logicas proc, fork y wait
//funcion ejec solo por proc hijo
static void similar_actividad_hijo(Actividad *act){
	//hijo cierra lectura de pipe
	close(act->pipe_fd[0]);

	printf("[HIJO-PID %d] Iniciando actividad '%s' (%s) - Tiempo: %d ms\n",
	getpid(), act->id, act->nombre, act->tiempo_ms);
	
	usleep(act->tiempo_ms*1000);
	//para simular fallo (15% de prob)
	if((rand() % 100)<15){
		printf("[HIJO-PID %d] err: actividad '%s' (%s) valió madres \n",
			getpid(), act->id, act->nombre);
		close(act->pipe_fd[1]);
		exit(EXIT_FAILURE); //termina codigo de error para que padre detecte
	}

	//hijo avisa por pipe
	char mensaje[256];
	snprintf(mensaje, sizeof(mensaje), "Actividad %s termino con exito", act->id);
	write(act->pipe_fd[1], mensaje, strlen(mensaje) + 1);

	//cierra write
	close(act->pipe_fd[1]);
	printf("[HIJO-PID %d] Finalizo actividad '%s'\n", getpid(), act->id);
	exit(EXIT_SUCCESS);
}
volatile sig_atomic_t llegada_seremi = 0;

void manejador_sigint(int sig){
	(void)sig; //evitar warning de parametro sin uso
	llegada_seremi=1;
}
//func para cancelar toda la rama descendiente de un fallo
void cancelar_sucesores(Planificador *plan, int idx_padre, int *actividades_finalizadas){
	Actividad *padre= &plan->actividades[idx_padre];
	for(int i=0; i<padre->num_hijos; i++){
		int idx_hijo=padre->hijos_indices[i];
		Actividad *hijo= &plan->actividades[idx_hijo];
		if(hijo->estado==ESTADO_PENDIENTE){
		hijo->estado=ESTADO_CANCELADO;
		(*actividades_finalizadas)++; //sumar tarea cancelada para que bucle avance
		printf("[SISTEMA] Actividad '%s' cancelada por fallo en su dependencia \n", hijo->id);
		//llamada para propagar cancelacion en cadena
		cancelar_sucesores(plan, idx_hijo, actividades_finalizadas);
		}
	}
}
// bucle principal ejec de padre
void ejecutar_planificador(Planificador *plan, int max_concurrencia_k){
	if(plan==NULL || max_concurrencia_k <= 0)return;
	//registrar manejador de señal antes de bucle
	signal(SIGINT, manejador_sigint);

	int procesos_activos=0;
	int actividades_finalizadas=0;
	printf("\n iniciando planificador (límite K = %d) \n\n", max_concurrencia_k);
	
	while(actividades_finalizadas<plan->total_actividades){
		//revisar si llega seremi ctrl c
		if(llegada_seremi){
			printf("\n[SEREMI] inspeccion detectada ctrl c, abortando actividades\n");
			for(int i=0;i<plan->total_actividades;i++){
			if(plan->actividades[i].estado==ESTADO_EN_EJECUCION && plan->actividades[i].pid> 0){
				kill(plan->actividades[i].pid, SIGKILL);
				printf("[SISTEMA] Proc %d (Actividad '%s') eliminado \n",
				plan->actividades[i].pid, plan->actividades[i].id);
			}
			}
			break; //romper bucle principal y terminar planificador
		}
		//que tareas pueden iniciar
		for(int i=0; i<plan->total_actividades;i++){
			Actividad *act= &plan->actividades[i];
			if(act->in_degree==0 && act->estado == ESTADO_PENDIENTE && procesos_activos < max_concurrencia_k){
			act->estado=ESTADO_EN_EJECUCION;
			
			//abrir pipe
			if(pipe(act->pipe_fd)==-1){
				perror("error en pipe");
				act->estado=ESTADO_PENDIENTE;
				continue;
			}
			pid_t pid=fork();
			if(pid < 0){
				perror("Error en fork");
				act->estado=ESTADO_PENDIENTE;
			}else if(pid==0){
			//codigo hijo
				similar_actividad_hijo(act);
			}else{
			//codigo padre
			//padre cierra write de pipe de este hijo
			close(act->pipe_fd[1]);
			act->pid = pid;
			procesos_activos++;
			}
		}
	}
	//esperar que termine algun hijo
	if(procesos_activos >0){
		int status;
		pid_t pid_terminado = waitpid(-1, &status, WNOHANG); //WNOHANG para no bloquear SIGINT

		if(pid_terminado>0){
			procesos_activos--;
			actividades_finalizadas++;
			
			for(int i =0; i<plan->total_actividades; i++){
				Actividad *act=&plan->actividades[i];
				if(act->pid==pid_terminado){
					//termino con error?
					if(WIFEXITED(status) && WEXITSTATUS(status)!=EXIT_SUCCESS){
					printf("[FALLO] la actividad '%s' falló. Cancelando rama descendiente\n", act->id);
					act->estado=ESTADO_FINALIZADA;
					cancelar_sucesores(plan, i, &actividades_finalizadas);
					close(act->pipe_fd[0]);
					}
					//terminó joya, flujo normal
					else if(WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS){
					act->estado=ESTADO_FINALIZADA;			
					//leer mensaje y cerrar lectura
					char buffer[256]={0};
					read(act->pipe_fd[0], buffer, sizeof(buffer));
					printf("[PADRE] mensaje recibido del PID %d: '%s'\n", pid_terminado, buffer);
					close(act->pipe_fd[0]);

					//desbloq a hijos reduciendo su in_degree
					for(int j=0; j<act->num_hijos;j++){
						int idx_hijo = act->hijos_indices[j];
						plan->actividades[idx_hijo].in_degree--;
					}
					}
					break;
				}
			}
		}
	}
}

printf("\n todas actividades finalizaron \n");
}
