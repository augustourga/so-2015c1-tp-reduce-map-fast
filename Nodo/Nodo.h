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
#include <utiles/messages.h>
#include <utiles/auxiliares.h>
#include <utiles/files.h>
#include "ejecuta_script.h"
/******************Constantes********************/
#define PATH "/home/utnso/git/tp-2015-1c-milanesa/Nodo/NODO_CONFIG.config"
#define LOG_FILE "/home/utnso/git/tp-2015-1c-milanesa/Nodo/NODO_LOG.txt"
#define PROCESO "Nodo"
#define BUFF_SIZE 1024
#define DISCO "/home/utnso/miarchivo.bin"
#define RUTA "/home/utnso/git/tp-2015-1c-milanesa/Nodo"
/****************Registros***********************/
typedef struct{
	int sock_fs;
}t_conexion_nodo;

typedef struct{
	int sock_fs;
}t_bloque_content;

typedef struct{
	int puerto;
	char* ip;
	char* archivos;
}t_nodo_archivo ;

/******************Variables************************/
		 int tamanio_bloque= 20*1024*1024;
         uint16_t PUERTO_FS;
		 char* IP_FS;
		 char* ARCHIVO_BIN;
		 char* DIR_TEMP;
		 int NODO_NUEVO;
		 char* IP_NODO;
	     int PUERTO_NODO;
	     int NODO_ID;
	     t_log* Log_Nodo;
         int rcx;
         char* _data;
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
char* crear_Espacio_Datos(int , char* , char* );
t_msg_id ejecutar_map(char*ejecutable,char*bloque,char* nombreArchivo);
t_msg_id ejecutar_reduce(char*ejecutable,char*archivo_final,t_list* listaArchivos);
#endif /* NODO_NODO_H_ */
