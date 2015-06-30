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

#define RUTA_LOG "log/marta.log"
#define PUERTO_LISTEN 3333

char* getRandName(char* str1, char* str2);
int conectarse_a_mdfs(char* ip_mdfs, uint16_t puerto_mdfs);

typedef struct {
	char* ip;
	int puerto;
	int numero_bloque;
} t_nodo;

typedef struct {
	char* ip;
	int puerto;
	int numero_bloque;
} t_nodo_global;


#endif /* MARTA_H_ */
