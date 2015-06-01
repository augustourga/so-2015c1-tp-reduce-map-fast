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
#include "Job.h"
#include <commons/config.h>
#include <commons/collections/dictionary.h>

/*********Constantes*****/
#define CONFIG_PATH "/home/utnso/git/tp-2015-1c-milanesa/Job/Job.config"
#define LOG_FILE "/home/utnso/Job_log.txt"
#define PROCESO "Job"
#define MAXSIZE 1024

typedef struct
{
	FILE * stream;
	struct stat * fstat;
} t_Datos_archivo;

typedef struct
{
	char* ip_marta;
	int puerto_marta;
	char** archivos;
	char* resultado;
	int combiner; //1 indica SI, 0 indica NO
	t_Datos_archivo* map;
	t_Datos_archivo* reduce;
} t_Datos_configuracion;


int obtenerConfiguracion();
int levantarArchivo(char*,t_Datos_archivo*);
void loguearLinea(char* linea,t_log_level);
void mostrarPorPantalla();
int conexionMaRTA();

/*********Variables globales******************/
t_Datos_configuracion* configuracion;
t_log* Log_job;
int marta_sock;

/********************************************/



#endif /* JOB_JOB_H_ */
