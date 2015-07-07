/*
 * job.h
 *
 *  Created on: 14/6/2015
 *      Author: utnso
 */

#ifndef SRC_TAREA_H_
#define SRC_TAREA_H_

#include <commons/collections/list.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct {
	char* ip;
	char* nombre;
	int puerto;
	int numero_bloque;
} t_nodo;


typedef struct {
	t_nodo nodo;
	char* nombre;
} t_temp;

typedef struct {
	char* nombre;
	int bloque;
	t_nodo copias[3];
} t_archivo;

typedef struct {
	int id;
	t_archivo archivo;
	t_temp arch_tmp;
} t_map;

typedef struct {
	int id;
	t_list* temporales;
} t_reduce;

t_map* map_crear();
t_map* reduce_crear();

#endif /* SRC_TAREA_H_ */
