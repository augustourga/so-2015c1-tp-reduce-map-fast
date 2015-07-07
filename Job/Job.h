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
#include <commons/log.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/collections/dictionary.h>
#include <commons/string.h>
#include <commons/collections/queue.h>
#include <utiles/messages.h>
#include <utiles/files.h>
#include <utiles/sockets.h>

/*********Constantes*****/
#define CONFIG_PATH "/home/augusto/workspace/tp-2015-1c-milanesa/Job/Job.config"
#define LOG_FILE "/home/utnso/Job_log.txt"
#define PROCESO "Job"
#define MAXSIZE 1024


typedef struct
{
	char* ip_marta;
	uint16_t puerto_marta;
	char* archivos;
	char* resultado;
	int combiner; //1 indica SI, 0 indica NO
	char* mapper;
	char* reduce;
} t_Datos_configuracion;

typedef struct
{
	char ip[15];
	uint16_t puerto;
	char* archivo_final;
	int id_operacion;
	int bloque;
} t_params_hiloMap;

typedef struct
{
	char ip[15];
	uint16_t puerto;
	char* nombre_nodo;
	char* archivo_final;
	int id_operacion;
	t_queue* archivos_tmp;
} t_params_hiloReduce;


int obtenerConfiguracion();
void loguearLinea(char* linea,t_log_level);
void mostrarPorPantalla();
int conexionMaRTA();
int HiloMap(void*);
int HiloReduce(void*);
int handshake_marta();
int levantar_hilo_mapper(t_params_hiloMap* nodo);
int levantar_hilo_reduce(t_params_hiloReduce* nodo);

/*********Variables globales******************/
t_Datos_configuracion* configuracion;
t_log* Log_job;
pthread_t* th_hilo_map;
pthread_t* th_hilo_reduce;
t_msg * mensaje_actual;
int marta_sock;
pthread_mutex_t * marta_mutex;

/********************************************/



#endif /* JOB_JOB_H_ */
