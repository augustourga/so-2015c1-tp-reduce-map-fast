/*
 * job.h
 *
 *  Created on: 6/7/2015
 *      Author: utnso
 */

#ifndef SRC_JOB_H_
#define SRC_JOB_H_

#include <commons/collections/list.h>
#include <utiles/messages.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include "tarea.h"

typedef struct {
	int id;
	int socket;
	char* archivo_final;
	bool combiner;
	t_list* maps;
	t_list* reduces; //Sólo se usa si es con combiner
	t_reduce* reduce_final;
	sem_t sem_maps_fin;
	sem_t sem_reduces_fin;
	sem_t sem_reduce_final_fin;
} t_job;

t_job* job_crear();
t_archivo solicitar_informacion_archivo(char* ruta_mdfs);
int id_job_next();
void ejecuta_maps(t_job* job);
void ejecuta_reduces_parciales(t_job* job);
void ejecuta_reduce_final(t_job* job);

#endif /* SRC_JOB_H_ */
