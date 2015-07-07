#include "server.h"


int socket_mdfs;

void iniciar_server(uint16_t puerto_listen) {

	fd_set master;    // master file descriptor list
	fd_set read_fds;  // temp file descriptor list for select()
	int fdmax;        // maximum file descriptor number
	int newfd;        	   // newly accept()ed socket descriptor
	int socket_actual;

	struct sockaddr_storage addr_client; // client address
	socklen_t addrlen;

	FD_ZERO(&master);    // clear the master and temp sets
	FD_ZERO(&read_fds);

	// Crea un socket de escucha y lo pone
	int listen_socket = server_socket(puerto_listen);

	// add the listener to the master set
	FD_SET(listen_socket, &master);

	// keep track of the biggest file descriptor
	fdmax = listen_socket; // so far, it's this one

	// main loop
	while (1) {
		read_fds = master; // copy it
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			log_error_consola("Falló el select");
			perror("Falló el select. Error");
			exit(3);
		}

		// run through the existing connections looking for data to read
		for (socket_actual = 0; socket_actual <= fdmax; socket_actual++) {
			if (FD_ISSET(socket_actual, &read_fds)) { // we got one!!
				if (socket_actual == listen_socket) {
					// handle new connections
					addrlen = sizeof addr_client;
					newfd = accept(listen_socket, (struct sockaddr *) &addr_client, &addrlen);
					if (newfd == -1) {
						log_error_consola("Falló el accept");
						perror("Falló el accept. Error");
					} else {
						if (newfd > fdmax) {    // keep track of the max
							fdmax = newfd;
						}
						t_msg* mensaje = recibir_mensaje(newfd);
						if (mensaje->header.id == CONEXION_JOB) {
							FD_SET(newfd, &master); // add to master set
							crearHiloJob(newfd, mensaje);
						}
						destroy_message(mensaje);
					}
				} else if (!socket_conectado(socket_actual)) {
					//manejar_desconexion(socket_actual); //TODO: Manejardesconexion
					FD_CLR(socket_actual, &master);
				} else {
					t_msg* mensaje = recibir_mensaje(socket_actual);
					decodificar_mensaje(mensaje, socket_actual);
				}
			}
		}
	}
}

void decodificar_mensaje(t_msg* mensaje, int socket) {

	switch (mensaje->header.id) {
	/*case FIN_MAP_OK: //TODO: Definir las respuestas
	 actualizar_tabla_tareas(mensaje->argv[0], FIN_MAP_OK);
	 break;
	 case FIN_MAP_ERROR:
	 actualizar_tabla_tareas_error(mensaje->argv[0], FIN_MAP_ERROR);
	 break;
	 case FIN_REDUCE_OK:
	 actualizar_tabla_tareas(mensaje->argv[0], FIN_REDUCE_OK);
	 break;
	 case FIN_REDUCE_ERROR:
	 actualizar_tabla_tareas_error(mensaje->argv[0], FIN_REDUCE_ERROR);
	 break;*/
	default:
		log_error_interno("Mensaje Incorrecto");
		break;
	}
}

int socket_conectado(int socket) {
	char buf[1];
	int bytes;
	if ((bytes = recv(socket, buf, 1, MSG_PEEK)) == 0) {
		close(socket);
	}
	return bytes;
}

void conectarse_a_mdfs(char* ip_mdfs, uint16_t puerto_mdfs) {

	if ((socket_mdfs = client_socket(ip_mdfs, puerto_mdfs)) < 0) {
		log_error_consola("Error al conectarse a MDFS");
		exit(1);
	}

	t_msg* mensaje = id_message(CONEXION_MARTA);
	enviar_mensaje(socket_mdfs, mensaje);

	destroy_message(mensaje);

}

char* get_info_archivo(char* ruta_mdfs) {

	char* ret;

	t_msg* message = string_message(INFO_ARCHIVO, ruta_mdfs, 0);
	enviar_mensaje(socket_mdfs, message);
	t_msg* respuesta = recibir_mensaje(socket_mdfs);

	if (respuesta->header.id == INFO_ARCHIVO_OK) {
		ret = string_duplicate(respuesta->stream);
	} else {
		log_error_consola("No se pudo realizar el job");
		ret = NULL;
	}

	destroy_message(respuesta);
	destroy_message(message);
	return ret;
}

void copiar_archivo_final() {

}

void crearHiloJob(int newfd, t_msg* mensaje) {
	struct arg_job args;

	pthread_t hilo_job;

	args.mensaje = mensaje;
	args.socket = newfd;

	pthread_create(&hilo_job, NULL, (void*) procesar_job, &args);
}

