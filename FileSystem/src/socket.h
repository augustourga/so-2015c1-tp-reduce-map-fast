/*
 * socket.h
 *
 *  Created on: 11/6/2015
 *      Author: utnso
 */

#ifndef SOCKET_H_
#define SOCKET_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <utiles/log.h>
#include "paquete.h"

#define BACKLOG 10			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo

void socket_bind(int listen_socket, struct addrinfo* server_info);
int socket_listen(char* puerto_listen);
int socket_conectado(int socket);

#endif /* SOCKET_H_ */
