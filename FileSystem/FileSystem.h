/*
 * FileSystem.h
 *
 *  Created on: 1/5/2015
 *      Author: utnso
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <pthread.h>
#include <unistd.h>
#include <libio.h>
#include <string.h>
#include <sockets.h>
#include <utiles/utiles.h>
#include <sys/types.h>
//#include <db.h>
#include <utiles/utiles.h>

#define BUFF_SIZE 1024
#define	DATABASE "directorios.db"

typedef struct {
	char *nombre_archivo;
	int tamanio;
	int directorio_padre;
	bool estado;
	t_dictionary bloques;
} t_archivo;

typedef struct {
	int id;
	int socket;
	bool conectado_andando;
	int cantidad_bloques_libres;
	t_dictionary bloques;
} t_nodo;

int main(void);

#endif /* FILESYSTEM_H_ */
