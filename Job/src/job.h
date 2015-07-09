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
#include <utiles/log.h>

/*********Constantes*****/
#define LOG_FILE "log/job.log"
#define PROCESO "Job"

typedef struct {
	char* ip_marta;
	uint16_t puerto_marta;
	char* archivos;
	char* resultado;
	int combiner; //1 indica SI, 0 indica NO
	char* mapper;
	char* reduce;
} t_Datos_configuracion;

typedef struct {
	char ip[15];
	uint16_t puerto;
	char* archivo_final;
	int id_operacion;
	int bloque;
} t_params_hiloMap;

typedef struct {
	char ip[15];
	uint16_t puerto;
	char* nombre_nodo;
	char* archivo_final;
	int id_operacion;
	t_queue* archivos_tmp;
} t_params_hiloReduce;

void obtenerConfiguracion();
void conectarseAMarta();
void esperarTareas();
int hiloMap(void*);
int hiloReduce(void*);
void handshakeMarta();
void levantarHiloMapper(t_params_hiloMap* nodo);
void levantarHiloReduce(t_params_hiloReduce* nodo);

/*********Variables globales******************/
t_Datos_configuracion* configuracion;
int marta_sock;
/********************************************/

#endif /* JOB_JOB_H_ */
