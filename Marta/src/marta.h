/*
 * Marta.h
 *
 *  Created on: 1/5/2015
 *      Author: utnso
 */

#ifndef MARTA_H_
#define MARTA_H_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <utiles/messages.h>
#include <utiles/log.h>
#include "config.h"
#include "server.h"
#include "tarea.h"
#include "job.h"

#define RUTA_LOG "log/marta.log"
#define PUERTO_LISTEN 3333

typedef struct {
	char* ip;
	char* nombre;
	int puerto;
	int carga_trabajo;
} t_nodo_global;

char* getRandName(char* str1, char* str2);
char* get_info_archivo(char* ruta_mdfs);
t_nodo get_nodo_menos_cargado(t_nodo nodos[3]);
void planificar_maps(t_job** job);
void agregar_nodo_si_no_existe(t_nodo nodo_nuevo);

void lista_jobs_add(t_job* job);
void lista_nodos_add(t_nodo_global* nodo);

#endif /* MARTA_H_ */
