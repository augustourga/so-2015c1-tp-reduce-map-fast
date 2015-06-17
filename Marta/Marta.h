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

#define MAXEVENTS 64
#define PUERTO 9999

int HacerMagiaConArchivo(char*);
void realizar_handshake_job(int);
void conexion_job(int sock_job);
void receive_messages_epoll(void);
void make_socket_non_blocking(int sfd);
int ejecutar_mensaje(int sock_fd, t_msg *recibido);

char* nombre_archivo;
bool alive;


#endif /* MARTA_H_ */
