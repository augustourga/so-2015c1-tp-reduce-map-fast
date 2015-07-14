/*
 * sockets.h
 *
 *  Created on: 17/6/2015
 *      Author: utnso
 */

#ifndef UTILES_SOCKETS_H_
#define UTILES_SOCKETS_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"

/****************** FUNCIONES SOCKET. ******************/

/*
 * Crea, vincula y escucha un socket desde un puerto determinado.
 */
int server_socket(uint16_t port);

/*
 * Crea y conecta a una ip:puerto determinado.
 */
int client_socket(char* ip, uint16_t port);

/*
 * Acepta la conexion de un socket.
 */
int accept_connection(int sock_fd);

/*
 * Convierte al socket en no bloqueante
 */
int fd_set_blocking(int fd, int blocking);

#endif /* UTILES_SOCKETS_H_ */
