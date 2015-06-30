/*
 * server.h
 *
 *  Created on: 18/5/2015
 *      Author: utnso
 */

#ifndef SRC_SERVER_H_
#define SRC_SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <errno.h>
#include <utiles/messages.h>
#include <utiles/log.h>

#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

struct arg_job {
	int socket;
	t_msg* mensaje;
};


void iniciar_server(uint16_t puerto_listen);
void decodificar_mensaje(t_msg* mensaje, int socket);
int socket_conectado(int socket);

#endif /* SRC_SERVER_H_ */
