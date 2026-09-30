#ifndef PLANIFICADOR_H
#define PLANIFICADOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

//estados posibles actividad
typedef enum{
	ESTADO_PENDIENTE,
	ESTADO_EN_EJECUCION,
	ESTADO_FINALIZADA,
	ESTADO_FALLIDA
} EstadoActividad;

//Nodo(actividad) del grafo DAG

typedef struct{

	char id[64]; //tamaños nomas por poner espacio suficiente
	char nombre[128];
	int tiempo_ms;
	//dependencias
	int in_degree; //cant depend no resueltas aun
	int num_dependencias;
	//lista nodos (hijos)
	int *hijos_indices; //arr indice hijos
	int num_hijos;
	int capacidad_hijos; //controla tamaño memoria asignada a hijos_indices
	//control procs y comunicacion
	pid_t pid;
	EstadoActividad estado;

	//pipe
	int pipe_fd[2];  // 0=lecture, 1=escritura
}Actividad;
//para almacenar el grafo
typedef struct{
	Actividad *actividades; //arr actividades
	int total_actividades; //max 10000
	int capacidad; //capacidad arr
}Planificador;
//func tiempo aleatorio si no viene dado (100-5000ms)
int obtener_tiempo_aleatorio(void);

Planificador* crear_planificador(int capacidad_inicial);
int cargar_planificador(Planificador *plan, const char *ruta_archivo);
void liberar_planificador(Planificador *plan);

#endif //PLANIFICADOR_H
