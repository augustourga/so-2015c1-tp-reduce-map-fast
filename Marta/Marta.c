/*
 * Marta.c
 *
 *  Created on: 1/5/2015
 *      Author: utnso
 */

#include "Marta.h"

int main(void) {

	receive_messages_epoll();

	return 0;
}

int HacerMagiaConArchivo(char* arch) {
	printf("NOMBRE ARCHIVO RECIBIDO:%s/n", arch);
	nombre_archivo = (char*) malloc(strlen(arch) + 1);
	strcpy(nombre_archivo, arch);
	return 0;

}

void realizar_handshake_job(int sock_job) {
	t_msg* mensaje;

	mensaje = recibir_mensaje(sock_job);

	while (mensaje->header.id != FIN_ENVIO_ARCH) {
		HacerMagiaConArchivo(mensaje->stream);
		destroy_message(mensaje);
		mensaje = recibir_mensaje(sock_job);

	}
	destroy_message(mensaje);
	enviar_mensaje(sock_job, id_message(FIN_ENVIO_ARCH));
	destroy_message(mensaje);
}

void receive_messages_epoll(void) {
	alive = 1;
	struct epoll_event event;
	struct epoll_event *events;

	memset(&event, 0, sizeof(event));

	int sfd = server_socket(9999);
	if (sfd < 0) {
//		log_error(logger_old, "No se pudo crear socket escucha.");
		perror("receive_messages_epoll");
		exit(EXIT_FAILURE);
	}

	int efd = epoll_create1(0);
	if (efd == -1) {
		perror("epoll_create");
		exit(EXIT_FAILURE);
	}

	event.data.fd = sfd;
	event.events = EPOLLIN;

	int s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
	if (s == -1) {
		perror("epoll_ctl");
		exit(EXIT_FAILURE);
	}

	/* Buffer where events are returned. */
	events = calloc(MAXEVENTS, sizeof event);

	/* The event loop. */
	while (alive) {
		int n, i;

		n = epoll_wait(efd, events, MAXEVENTS, -1);
		for (i = 0; i < n; i++) {
			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))) {
				/* An error has occured on this fd, or the socket is not ready for reading. */
				perror("epoll error");
				close(events[i].data.fd);
			} else if (sfd == events[i].data.fd) {
				/* We have a notification on the listening socket, which means one incoming connection. */
				int infd = accept_connection(sfd);

				/* Make the incoming socket non-blocking and add it to the list of fds to monitor. */
//				make_socket_non_blocking(infd);
				event.data.fd = infd;
				event.events = EPOLLIN;

				s = epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event);
				if (s == -1) {
					perror("epoll_ctl");
					exit(EXIT_FAILURE);
				}
			} else {
				/* We have data on the fd waiting to be read. */

				t_msg *msg = recibir_mensaje(events[i].data.fd);
				if (msg == NULL) {
//					int status = remove_from_lists(events[i].data.fd);

					/* Closing the descriptor will make epoll remove it from the set of descriptors which are monitored. */
					close(events[i].data.fd);

//					if (status == -1) {
//						/* Exit program. */
//						free(events);
//						close(sfd);
//						return;
//					}
				} else
					ejecutar_mensaje(events[i].data.fd, msg);
			}
		}
	}

	free(events);
	close(sfd);
}

void conexion_job(int sock_job) {
	t_msg* mensaje;
	realizar_handshake_job(sock_job);

	while (1) {
		enviar_mensaje(sock_job, mensaje = string_message(EJECUTAR_MAP, "127.0.0.1", 1, 6545));
		destroy_message(mensaje);
		mensaje = string_message(ARCHIVO_JOB_MAP, nombre_archivo, 0);
		enviar_mensaje(sock_job, mensaje);
		mensaje = recibir_mensaje(sock_job);
		print_msg(mensaje);
	}

}

int ejecutar_mensaje(int sock_fd, t_msg *recibido) {
	switch (recibido->header.id) {
	case CONEXION_JOB:
		conexion_job(sock_fd);
		break;
	default:
		return -1;
		break;
	}
	return 0;
}
