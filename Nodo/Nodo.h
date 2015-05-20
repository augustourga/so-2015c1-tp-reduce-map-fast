/*
 * Nodo.h
 *
 *  Created on: 25/4/2015
 *      Author: utnso
 */

#ifndef NODO_NODO_H_
#define NODO_NODO_H_


#include <stdio.h>
#include <unistd.h>
#include <libio.h>
#include <string.h>
#include <stdlib.h>
#include "sockets.h"
#include <commons/config.h>
#include <commons/log.h>
#include <pthread.h>
#include "serializacion.h"


/******************Constantes********************/
#define PATH "/home/utnso/git/tp-2015-1c-milanesa/Nodo/NODO_CONFIG.config"
#define LOG_FILE "/home/utnso/Job_log.txt"
#define PROCESO "Nodo"
#define BUFF_SIZE 1024
/****************Registros***********************/
typedef struct{
	int sock_fs;
}t_conexion_nodo;

typedef struct{
	int sock_fs;
}t_bloque_content;

/******************Variables************************/
		 int PUERTO_FS;
		 char* IP_FS;
		 char* ARCHIVO_BIN;
		 char* DIR_TEMP;
		 char* NODO_NUEVO;
		 char* IP_NODO;
	     int PUERTO_NODO;
	     t_log* Log_Nodo;
         int rcx;
         char* NOMBRE_NODO = "NODO_PIOLA";
/******************Definiciones*******************/

int levantarConfiguracionNodo();
void conectarFileSystem();
 //getBloque(unNumero);
int levantarServer();
int levantarHiloFile();


#endif /* NODO_NODO_H_ */
