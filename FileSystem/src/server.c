#include "server.h"

t_list* lista_get_bloque;
t_list* lista_set_bloque;

void iniciar_server(void* argumentos) {

	lista_get_bloque = list_create();
	lista_set_bloque = list_create();

	struct arg_struct *args = argumentos;

	char* puerto_listen = args->puerto_listen;

	fd_set master;    // master file descriptor list
	fd_set read_fds;  // temp file descriptor list for select()
	int fdmax;        // maximum file descriptor number
	int newfd;        	   // newly accept()ed socket descriptor
	int socket_actual;
	int socket_marta = 0;
	int cant_bytes;

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
						FD_SET(newfd, &master); // add to master set
						if (newfd > fdmax) {    // keep track of the max
							fdmax = newfd;
						}

						t_paquete* mensaje = socket_recibir(newfd, &cant_bytes);
						if (mensaje->cod_op == MARTA_HANDSHAKE) {
							socket_marta = newfd;
						}
						decodificar_mensaje(mensaje, newfd);
						free(mensaje);
					}
				} else if (!socket_conectado(socket_actual)) {
					manejar_desconexion(socket_actual);
					FD_CLR(socket_actual, &master);
				} else if (socket_actual == socket_marta) {
					t_paquete* mensaje = socket_recibir(newfd, &cant_bytes);
					decodificar_mensaje(mensaje, newfd);
					free(mensaje);
				}
			}
		}
	}
}

void decodificar_mensaje(t_paquete* mensaje, int socket) {
	extern int filesystem_operativo;

	switch (mensaje->cod_op) {
	case NODO_HANDSHAKE:
		registrar_nodo(nodo_deserealizar_socket(mensaje->data, socket));
		break;
	case MARTA_HANDSHAKE:
		if (filesystem_operativo) {

		}
		break;
	case INFO_ARCHIVO:
		if (filesystem_operativo) {
			//char* respuesta = preparar_info_archivo(mensaje); //TODO: preparar_info_archivo
			//socket_send_all(socket, respuesta);
		}
		break;
	case COPIAR_ARCHIVO:
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
	int cant_bytes;
	int offset = 0;
	int bytes_a_copiar;

	t_paquete* paquete_solicitud = mensaje_serializado_get_bloque(bloque);
	socket_send_all(socket, paquete_solicitud);
	t_paquete* respuesta = socket_recibir(socket, &cant_bytes);

	switch (respuesta->cod_op) {
	case GET_BLOQUE_FAIL:
		log_error_interno("No se pudo obtener el bloque");
		return stream_bloque;
		break;
	case GET_BLOQUE_OK:
		stream_bloque = malloc(respuesta->largo_data);
		bytes_a_copiar = respuesta->largo_data;
		memcpy(stream_bloque, respuesta->data + offset, bytes_a_copiar);
		free(respuesta->data);

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
	int largo_chunk = args->largo_chunk;
	char* chunk = args->chunk;
	int cant_bytes;

	t_paquete* paquete_solicitud = mensaje_serializado_set_bloque(bloque, chunk, largo_chunk);
	socket_send_all(socket, paquete_solicitud);
	t_paquete* respuesta = socket_recibir(socket, &cant_bytes);

	switch (respuesta->cod_op) {
	case SET_BLOQUE_FAIL:
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

t_paquete* mensaje_serializado_get_bloque(int numero_bloque) {

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->cod_op = GET_BLOQUE;
	paquete->largo_data = sizeof(numero_bloque);

	paquete->data = malloc(paquete->largo_data);
	memcpy(paquete->data, &numero_bloque, sizeof(numero_bloque));

	return paquete;

}

t_paquete* mensaje_serializado_set_bloque(int numero_bloque, char* chunk, int largo_chunk) {

	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->cod_op = SET_BLOQUE;
	paquete->largo_data = sizeof(numero_bloque) + sizeof(largo_chunk) + largo_chunk;

	paquete->data = malloc(paquete->largo_data);

	int offset = 0;

	paquete_serializar(paquete->data, &numero_bloque, sizeof(numero_bloque), &offset);

	paquete_serializar(paquete->data, &largo_chunk, sizeof(largo_chunk), &offset);

	paquete_serializar(paquete->data, chunk, largo_chunk, &offset);

	return paquete;
}

