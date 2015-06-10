/*
 * Job.h
 *
 *  Created on: 25/4/2015
 *      Author: utnso
 */

#ifndef JOB_JOB_H_
#define JOB_JOB_H_

#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sockets.h>
#include <commons/log.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>
#include <utiles/utiles.h>

/*********Constantes*****/
#define CONFIG_PATH "/home/utnso/git/tp-2015-1c-milanesa/Job/Job.config"
#define LOG_FILE "/home/utnso/Job_log.txt"
#define PROCESO "Job"
#define MAXSIZE 1024


typedef struct
{
	char* ip_marta;
	int puerto_marta;
	char** archivos;
	char* resultado;
	int combiner; //1 indica SI, 0 indica NO
	char* mapper;
	char* reduce;
	int cant_archivos;
} t_Datos_configuracion;

typedef struct
{
	char ip[15];
	int puerto;
} t_direccion_proceso;


int obtenerConfiguracion();
void loguearLinea(char* linea,t_log_level);
void mostrarPorPantalla();
int conexionMaRTA();
int HiloMap(void*);
int HiloReduce(void*);
int enviar_mensaje_inicial_marta();
int levantar_hilo_mapper(t_direccion_proceso* nodo);
int levantar_hilo_reduce(t_direccion_proceso* nodo);

/*********Variables globales******************/
t_Datos_configuracion* configuracion;
t_log* Log_job;
pthread_t* th_hilo_map;
pthread_t* th_hilo_reduce;
t_msg * mensaje_actual;
int marta_sock;

/********************************************/



#endif /* JOB_JOB_H_ */
