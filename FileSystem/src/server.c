#include "server.h"

extern pthread_mutex_t mutex_args;

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
					if (socket_actual == socket_marta) {
						desconexion_marta(socket_actual);
					} else {
						desconexion_nodo(socket_actual);
					}
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
		log_info_interno("Marta se conectó correctamente. Su socket es %d",
				socket);
		break;
	case INFO_ARCHIVO:
		if (filesystem_operativo) {
			t_msg* msg_rta = mensaje_info_archivo(mensaje->stream);
			enviar_mensaje(socket, msg_rta);
		} else {
			enviar_fs_no_operativo(socket);
		}
		break;
	case GET_ARCHIVO_TMP:
		if (filesystem_operativo) {
			t_msg* msg_rta = mensaje_copiar_archivo_temporal_a_mdfs(mensaje->stream);
			enviar_mensaje(socket, msg_rta);
		} else {
			enviar_fs_no_operativo(socket);
		}
		break;
	default:
		log_error_interno("Mensaje Incorrecto: ", mensaje->header.id);
		break;
	}
}

void desconexion_nodo(int socket) {
	desconectar_nodo(socket);
}

void desconexion_marta(int socket) {
	desconectar_marta(socket);
}

void enviar_fs_no_operativo(int socket) {
	t_msg* msg = id_message(MDFS_NO_OPERATIVO);
	enviar_mensaje(socket, msg);
	destroy_message(msg);
}

void* mensaje_get_bloque(void* argumentos) {

	struct arg_get_bloque* args = argumentos;

	int bloque = args->bloque_nodo;
	int bloque_archivo = args->bloque_archivo;
	int socket = args->socket;

	t_msg* msg_solicitud = argv_message(GET_BLOQUE, 2, bloque, bloque_archivo);
	enviar_mensaje(socket, msg_solicitud);
	destroy_message(msg_solicitud);
	pthread_mutex_unlock(&mutex_args);
	t_msg* respuesta = recibir_mensaje(socket);
	struct res_get_bloque* res = malloc(sizeof(struct res_get_bloque));

	switch (respuesta->header.id) {
	case GET_BLOQUE_ERROR:
		log_error_interno("No se pudo obtener el bloque");
		destroy_message(respuesta);
		return res;
		break;
	case GET_BLOQUE_OK:
		res->stream = malloc(respuesta->header.length);
		res->bloque_archivo = respuesta->argv[0];
		memcpy(res->stream, respuesta->stream, respuesta->header.length);
		destroy_message(respuesta);
		return res;
		break;
	default:
		log_error_interno("Respuesta Incorrecta");
		return res;
		break;
	}
}

int mensaje_set_bloque(void* argumentos) {
	log_debug_interno("Empieza set_bloque");
	struct arg_set_bloque* args = argumentos;

	int socket = args->socket;
	char* stream = malloc(args->chunk.tamanio);
	memcpy(stream, args->chunk.inicio, args->chunk.tamanio);
	t_msg* msg_solicitud = string_message(SET_BLOQUE, stream, 2, args->bloque_nodo, args->chunk.tamanio);
	enviar_mensaje(socket, msg_solicitud);
	destroy_message(msg_solicitud);
	free(stream);
	pthread_mutex_unlock(&mutex_args);
/*	t_msg* respuesta = recibir_mensaje(socket);

	switch (respuesta->header.id) {
	case SET_BLOQUE_ERROR:
		log_error_interno("No se pudo setear el bloque");
		return 1;
		break;
	case SET_BLOQUE_OK:
		log_debug_interno("Terminó set_bloque OK");
		// TODO: logear HDP!
		return 0;
		break;
	default:
		log_error_interno("Respuesta Incorrecta");
		return 2;
	}*/
	return 0;
}

t_msg* mensaje_info_archivo(char* ruta_archivo) {
	t_msg* respuesta;
	char* info;

	info = preparar_info_archivo(ruta_archivo);
	if (info != NULL) {
		respuesta = string_message(INFO_ARCHIVO_OK, info, 0);
	} else {
		respuesta = id_message(INFO_ARCHIVO_ERROR);
	}

	return respuesta;
}

t_msg* mensaje_copiar_archivo_temporal_a_mdfs(char* mensaje) {
	t_msg* respuesta;
	t_nodo* nodo;
	char* nombre_archivo_tmp = string_new();
	char* nombre_nodo = string_new();
	int valor;

	char** parametros = string_n_split(mensaje, 2, "|");
	string_append(&nombre_archivo_tmp, parametros[0]);
	string_append(&nombre_nodo, parametros[1]);
	nodo = nodo_operativo_por_nombre(nombre_nodo);
	if (nodo == NULL) {
		log_error_interno(
				"El nodo no se encuentra operativo para solicitarle un archivo temporal");
		return id_message(GET_ARCHIVO_TMP_ERROR);
	}

	t_msg* msg_solicitud = string_message(GET_FILE_CONTENT, nombre_archivo_tmp,
			0);
	enviar_mensaje(nodo->socket, msg_solicitud);
	t_msg* msg_respuesta = recibir_mensaje(nodo->socket);
	switch (msg_respuesta->header.id) {
	case GET_FILE_CONTENT_ERROR:
		log_error_interno("No se pudo obtener el archivo temporal del nodo");
		respuesta = id_message(GET_ARCHIVO_TMP_ERROR);
		break;
	case GET_FILE_CONTENT_OK:
		valor = copiar_archivo_temporal_a_mdfs(nombre_archivo_tmp, msg_respuesta->stream);
		if (valor) {
			log_info_interno("Pudo copiarse el archivo tmp a la raiz del mdfs con exito");
			respuesta = id_message(GET_ARCHIVO_TMP_OK);
		} else {
			log_error_interno("No se pudo copiar el archivo tmp al mdfs");
			respuesta = id_message(GET_ARCHIVO_TMP_ERROR);
		}
		break;
	default:
		log_error_interno("Respuesta Incorrecta");
		respuesta = id_message(GET_ARCHIVO_TMP_ERROR);
	}
	return respuesta;
}

