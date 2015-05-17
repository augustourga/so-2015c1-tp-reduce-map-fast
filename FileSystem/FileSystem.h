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
#include <sockets.h>
#include <pthread.h>
#include <serializacion.h>

typedef struct {
	char *nombre_archivo;
	int tamanio;
	int directorio_padre;
	bool estado;
	t_dictionary bloques;
} t_archivo;
int main(void);

#endif /* FILESYSTEM_H_ */
