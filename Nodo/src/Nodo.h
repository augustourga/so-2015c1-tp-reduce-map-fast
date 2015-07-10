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
#define PATH_CONFIG "files/NODO_CONFIG.config"
#define LOG_FILE "files/NODO_LOG.txt"
#define PROCESO "Nodo"
#define BUFF_SIZE 1024
#define DISCO "files/miarchivo.bin"
#define RUTA "/home/utnso/git/tp-2015-1c-milanesa/Nodo"
/****************Registros***********************/
typedef struct {
	int sock_fs;
} t_conexion_nodo;

typedef struct {
	int puerto;
	char* ip;
	char* archivo;
} t_nodo_archivo;

typedef struct {
	char* nombre_archivo;
	fpos_t* posicion_puntero;
} t_archivo_tmp;

/******************Variables************************/
int tamanio_bloque = 20 * 1024 * 1024;
uint16_t PUERTO_FS;
char* IP_FS;
char* ARCHIVO_BIN;
char* DIR_TEMP;
int NODO_NUEVO;
int PUERTO_NODO;
t_log* Log_Nodo;
char* _data;
int CANT_BLOQUES;
char* NOMBRE_NODO;
int rcx;
t_list* archivos_temporales;
/******************Definiciones*******************/
char* levantar_espacio_datos();
int levantarConfiguracionNodo();
void conectarFileSystem();
void levantarNuevoServer();
int levantarHiloFile();
void *atenderConexiones(void *parametro);
/******************FUNCIONALIDADES*******************/
void setBloque(int numeroBloque, char* bloque_datos);
char* getBloque(int numeroBloque);
char* getFileContent(char* filename);
char* generar_nombre_temporal(int mapreduce_id, char*map_o_reduce, int numeroBloque);
char* crear_Espacio_Datos(int, char*, char*);
void liberar_Espacio_datos(char* _data, char* ARCHIVO);
t_list* deserealizar_cola(t_queue* colaArchivos);
t_msg_id ejecutar_map(char*ejecutable, char* nombreArchivoFinal, int numeroBloque, int mapid);
t_msg_id ejecutar_reduce(char*ejecutable, char*archivo_final, t_queue* colaArchivos, int id_reduce);

/******************APAREO***************************/
int apareo(char* temporal, t_list* lista_nodos_archivos);
char* obtener_proximo_registro(t_nodo_archivo* nodo_archivo);
int obtener_posicion_menor_clave(char** registros, int cantidad_nodos_archivos);
char* obtener_proximo_registro_de_archivo(char* archivo);
char* enviar_mensaje_proximo_registro(t_nodo_archivo* nodo_archivo);
void list_add_archivo_tmp(char* nombre_archivo);
fpos_t* obtener_posicion_puntero_arch_tmp(char* nombre_archivo);
void actualizar_posicion_puntero_arch_tmp(char* nombre_archivo, fpos_t* posicion_puntero);

#endif /* NODO_NODO_H_ */
