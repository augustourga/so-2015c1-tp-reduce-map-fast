#include "server.h"

void iniciar_server(void* argumentos) {

	struct arg_struct *args = argumentos;

	char* puerto_listen = args->puerto_listen;

	fd_set master;    // master file descriptor list
	fd_set read_fds;  // temp file descriptor list for select()
	int fdmax;        // maximum file descriptor number
	int newfd;        	   // newly accept()ed socket descriptor
	int socket_actual;
	int socket_marta = 0;

	struct sockaddr_storage addr_client; // client address
	socklen_t addrlen;

	FD_ZERO(&master);    // clear the master and temp sets
	FD_ZERO(&read_fds);

	// Crea un socket de escucha y lo pone
	int listen_socket = socket_listen(puerto_listen);

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
		}

		// run through the existing connections looking for data to read
		for (socket_actual = 0; socket_actual <= fdmax; socket_actual++) {
			if (FD_ISSET(socket_actual, &read_fds)) { // we got one!!
				if (socket_actual == listen_socket) {
					// handle new connections
					addrlen = sizeof addr_client;
					newfd = accept(listen_socket,
							(struct sockaddr *) &addr_client, &addrlen);
					if (newfd == -1) {
						log_error_consola("Falló el accept");
						perror("Falló el accept. Error");
					} else {
						FD_SET(newfd, &master); // add to master set
						if (newfd > fdmax) {    // keep track of the max
							fdmax = newfd;
						}

						t_msg* mensaje = recibir_mensaje(newfd);
						if (mensaje->header.id == CONEXION_MARTA) {
							socket_marta = newfd;
						}
						decodificar_mensaje(mensaje, newfd);
						free(mensaje);
					}
				} else if (!socket_conectado(socket_actual)) {
					manejar_desconexion(socket_actual);
					FD_CLR(socket_actual, &master);
				} else if (socket_actual == socket_marta) {
					t_msg* mensaje = recibir_mensaje(newfd);
					decodificar_mensaje(mensaje, newfd);
					free(mensaje);
				}
			}
		}
	}
}

void decodificar_mensaje(t_msg* mensaje, int socket) {
	extern int filesystem_operativo;

	switch (mensaje->header.id) {
	case CONEXION_NODO:
		registrar_nodo(nodo_deserealizar_socket(mensaje, socket));
		break;
	case CONEXION_MARTA:
		//TODO: registrar marta!
		break;
	case INFO_ARCHIVO:
		if (filesystem_operativo) {
			//char* respuesta = preparar_info_archivo(mensaje); //TODO: preparar_info_archivo
			//socket_send_all(socket, respuesta);
		}
		break;
	case GET_FILE_CONTENT:
		if (filesystem_operativo) {
			//char* respuesta = copiar_archivo_temporal_a_mdfs(mensaje); //TODO: copiar_archivo_temporal_a_mdfs
			//socket_send_all(socket, respuesta);
		}
		break;
	default:
		log_error_interno("Mensaje Incorrecto");
		break;
	}
}

void manejar_desconexion(int socket) {
	desconectar_nodo(socket);
	desconectar_marta(socket);
}

char* mensaje_get_bloque(void* argumentos) {

	struct arg_get_bloque* args = argumentos;
	char* stream_bloque = NULL;

	int bloque = args->bloque_nodo;
	int socket = args->socket;
	int offset = 0;
	int bytes_a_copiar;

	t_msg* msg_solicitud = argv_message(GET_BLOQUE, 1, bloque);
	enviar_mensaje(socket, msg_solicitud);
	t_msg* respuesta = recibir_mensaje(socket);

	switch (respuesta->header.id) {
	case GET_BLOQUE_ERROR:
		log_error_interno("No se pudo obtener el bloque");
		return stream_bloque;
		break;
	case GET_BLOQUE_OK:
		stream_bloque = malloc(respuesta->header.length);
		bytes_a_copiar = respuesta->header.length;
		memcpy(stream_bloque, respuesta->stream + offset, bytes_a_copiar);
		free(respuesta->stream);

		return stream_bloque;
		break;
	default:
		log_error_interno("Respuesta Incorrecta");
		return stream_bloque;
		break;
	}
}

int mensaje_set_bloque(void* argumentos) {

	struct arg_set_bloque* args = argumentos;

	int bloque = args->bloque_nodo;
	int socket = args->socket;
	char* chunk = args->chunk;

	t_msg* msg_solicitud = string_message(SET_BLOQUE, chunk, 1, bloque);
	enviar_mensaje(socket, msg_solicitud);
	t_msg* respuesta = recibir_mensaje(socket);

	switch (respuesta->header.id) {
	case SET_BLOQUE_ERROR:
		log_error_interno("No se pudo obtener el bloque");
		return 1;
		break;
	case SET_BLOQUE_OK:
		return 0;
		break;
	default:
		log_error_interno("Respuesta Incorrecta");
		return 2;
	}
}
