/*
 * directorio.h
 *
 *  Created on: 22/5/2015
 *      Author: utnso
 */

#ifndef SRC_DIRECTORIO_H_
#define SRC_DIRECTORIO_H_

#include <db.h>
#include <string.h>
#include <stdlib.h>
#include <utiles/log.h>
#include "paquete.h"

typedef struct {
	db_recno_t id;
	char nombre[255];
	db_recno_t padreId;
	pthread_rwlock_t lock;
} t_directorio;

t_directorio* directorio_crear();
t_directorio* directorio_crear_raiz();
void directorio_set_nombre(t_directorio* directorio, char* nombre);
void directorio_set_padre(t_directorio* directorio, db_recno_t padreId);
void directorio_eliminar(t_directorio* directorio);
char* directorio_serializar(t_directorio* directorio);
t_directorio* directorio_deserealizar(char* directorio_serializado);
void log_error_ya_existe_directorio(char* directorio, char* directorio_padre_nuevo);
void log_error_directorio_no_existe(char* ruta_directorio);

#endif /* SRC_DIRECTORIO_H_ */
