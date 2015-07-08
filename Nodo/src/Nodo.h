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
#include <commons/config.h>
#include <commons/log.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <utiles/messages.h>
#include <utiles/auxiliares.h>
#include <utiles/files.h>
#include <utiles/log.h>
#include "ejecuta_script.h"

/******************Constantes********************/
#define PATH_CONFIG "../files/NODO_CONFIG.config"
#define LOG_FILE "../files/NODO_LOG.txt"
#define PROCESO "Nodo"
#define BUFF_SIZE 1024
#define DISCO "../files/miarchivo.bin"
#define RUTA "/home/utnso/git/tp-2015-1c-milanesa/Nodo"
/****************Registros***********************/
typedef struct{
	int sock_fs;
}t_conexion_nodo;


typedef struct{
	int puerto;
	char* ip;
	char* archivo;
	fpos_t* posicion_puntero;
}t_nodo_archivo ;


/******************Variables************************/
		 int tamanio_bloque= 20*1024*1024;
         uint16_t PUERTO_FS;
		 char* IP_FS;
		 char* ARCHIVO_BIN;
		 char* DIR_TEMP;
		 int NODO_NUEVO;
		 int PUERTO_NODO;
	     int NODO_ID;
	     t_log* Log_Nodo;
         char* _data;
         int CANT_BLOQUES;
         char* NOMBRE_NODO;
         int rcx;
/******************Definiciones*******************/

int levantarConfiguracionNodo();
void conectarFileSystem();
void levantarNuevoServer();
int levantarHiloFile();
void *atenderConexiones(void *parametro);
/******************FUNCIONALIDADES*******************/
void setBloque(int numeroBloque, char* bloque_datos);
char* getBloque(int numeroBloque);
char* getFileContent(char* filename);
char* generar_nombre_temporal( int mapreduce_id, char*map_o_reduce,int numeroBloque);
char* crear_Espacio_Datos(int , char* , char* );
void liberar_Espacio_datos(char* _data,char* ARCHIVO);
void apareo(char* temporal,t_queue* colaArchivos);
t_msg_id ejecutar_map(char*ejecutable,char* nombreArchivoFinal,int numeroBloque,int mapid);
t_msg_id ejecutar_reduce(char*ejecutable,char*archivo_final,t_queue* colaArchivos,int id_reduce);
#endif /* NODO_NODO_H_ */
