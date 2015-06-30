/*
 * job.h
 *
 *  Created on: 14/6/2015
 *      Author: utnso
 */

#ifndef SRC_JOB_H_
#define SRC_JOB_H_

#include "marta.h"
#include <commons/collections/list.h>

typedef struct {
	t_nodo nodo;
	char* nombre;
} t_temp;

typedef struct {
	char* archivo;
	int bloque_archivo;
	t_nodo nodo_1;
	t_nodo nodo_2;
	t_nodo nodo_3;
	bool map;
	t_temp map_temp_nombre;
	bool reduce;
	t_temp reduce_temp_nombre;
	int socket;
} t_job;

void procesar_job(void* argumentos);

#endif /* SRC_JOB_H_ */
