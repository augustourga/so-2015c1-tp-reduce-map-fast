/*
 * sockets.h
 *
 *  Created on: 19/09/2014
 *      Author: utnso
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <unistd.h>

int obtener_socket();
void conectar_socket(int, char*, int);
int crear_socket_listen(int);
void vincular_socket(int socket, int puerto);

#endif /* SOCKETS_H_ */
