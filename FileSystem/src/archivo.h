/*
 * archivo.h
 *
 *  Created on: 23/5/2015
 *      Author: utnso
 */

#ifndef SRC_ARCHIVO_H_
#define SRC_ARCHIVO_H_

#include <db.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <utiles/log.h>
#include "paquete.h"

#define CANTIDAD_COPIAS 3

typedef struct {
	char* inicio;
	int tamanio;
} t_chunk;

typedef struct {
	char nombre_nodo[80];
	int bloque_nodo;
	int tamanio_bloque;
	bool conectado;
} t_copia;

typedef struct {
	int cantidad_copias;
	t_copia* copias;
} t_bloque;

typedef struct {
	db_recno_t id;
	char nombre[255];
	long tamanio;
	db_recno_t padreId;
	int cantidad_bloques;
	int cantidad_copias_totales;
	t_bloque* bloques;
	bool disponible;
	pthread_rwlock_t lock;
} t_archivo;

t_archivo* archivo_crear();
void archivo_set_nombre(t_archivo* archivo, char* nombre);
void archivo_set_padre(t_archivo* archivo, db_recno_t padreId);
void archivo_set_tamanio(t_archivo* archivo, int tamanio);
void archivo_asignar_estado(t_archivo* archivo, bool estado);
char* archivo_serializar(t_archivo* archivo, int* bytes_a_serializar);
t_archivo* archivo_deserealizar(char* archivo_serializado);
void log_error_archivo_no_existe(char* ruta_local);

#endif /* SRC_ARCHIVO_H_ */
