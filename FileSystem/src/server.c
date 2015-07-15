#include "server.h"

extern sem_t sem_set_bloque;

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
					newfd = accept(listen_socket, (struct sockaddr *) &addr_client, &addrlen);
					if (newfd == -1) {
						log_error_consola("Falló el accept");
					} else {
						FD_SET(newfd, &master); // add to master set
						if (newfd > fdmax) {    // keep track of the max
							fdmax = newfd;
						}

						t_msg* mensaje = recibir_mensaje(newfd);
						if (mensaje == NULL) {
							log_error_consola("Error al recibir mensaje");
						} else {
							if (mensaje->header.id == CONEXION_MARTA) {
								socket_marta = newfd;
							}
							decodificar_mensaje(mensaje, newfd);
							destroy_message(mensaje);
						}
					}
				} else if (!socket_conectado(socket_actual)) {
					if (socket_actual == socket_marta) {
						desconexion_marta(socket_actual);
					} else {
						desconexion_nodo(socket_actual);
					}
					FD_CLR(socket_actual, &master);
				} else if (socket_actual == socket_marta) {
					t_msg* mensaje = recibir_mensaje(socket_actual);
					if (mensaje == NULL) {
						log_error_consola("Error al recibir mensaje de Marta");
					} else {
						decodificar_mensaje(mensaje, socket_actual);
						destroy_message(mensaje);
					}
				}
			}
		}
	}
}

void decodificar_mensaje(t_msg* mensaje, int socket) {
	extern bool filesystem_operativo;
	int res;
	log_debug_interno("Mensaje %s recibido y decodificandolo", id_string(mensaje->header.id));
	switch (mensaje->header.id) {
	case CONEXION_NODO:
		registrar_nodo(nodo_deserealizar_socket(mensaje, socket));
		break;
	case CONEXION_MARTA:
		log_debug_interno("Marta se conectó correctamente. Su socket es %d", socket);
		break;
	case INFO_ARCHIVO:
		if (filesystem_operativo) {
			t_msg* msg_rta = mensaje_info_archivo(mensaje->stream);
			res = enviar_mensaje(socket, msg_rta);
			if (res < 0) {
				log_error_consola("No se pudo enviar mensaje de %s a Marta. Se asume su desconexion", id_string(msg_rta->header.id));
				desconectar_marta(socket);
			}
			destroy_message(msg_rta);
		} else {
			enviar_fs_no_operativo(socket);
		}
		break;
	case GET_ARCHIVO_TMP:
		if (filesystem_operativo) {
			t_msg* msg_rta = mensaje_copiar_archivo_temporal_a_mdfs(mensaje->stream);
			res = enviar_mensaje(socket, msg_rta);
			if (res < 0) {
				log_error_consola("No se pudo enviar mensaje de %s a Marta. Se asume su desconexion", id_string(msg_rta->header.id));
				desconectar_marta(socket);
			}
			destroy_message(msg_rta);
		} else {
			enviar_fs_no_operativo(socket);
		}
		break;
	default:
		log_error_interno("Mensaje Incorrecto: %s", id_string(mensaje->header.id));
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
	int res;
	res = enviar_mensaje(socket, msg);
	if (res < 0) {
		log_error_consola("No se pudo enviar mensaje de %s a Marta. Se asume su desconexion", id_string(msg->header.id));
		desconectar_marta(socket);
	}
	destroy_message(msg);
}

void* mensaje_get_bloque(void* argumentos) {

	struct arg_get_bloque* args = argumentos;
	log_info_interno("Empieza get_bloque del bloque %d", args->bloque_nodo);
	int bloque = args->bloque_nodo;
	int bloque_archivo = args->bloque_archivo;
	int socket = args->socket;

	int ret;

	t_msg* msg_solicitud = argv_message(GET_BLOQUE, 2, bloque, bloque_archivo);
	ret = enviar_mensaje(socket, msg_solicitud);
	if (ret < 0) {
		log_error_consola("Mensaje get_bloque fallo por desconexion del nodo, socket: %d", socket);
		desconexion_nodo(socket);
		return NULL;
	}
	destroy_message(msg_solicitud);
	t_msg* respuesta = recibir_mensaje(socket);
	if (respuesta == NULL) {
		log_error_consola("Mensaje get_bloque fallo porque el nodo no respondió, se asume desconexion del nodo, socket: %d", socket);
		desconexion_nodo(socket);
		return NULL;
	}
	struct res_get_bloque* res = malloc(sizeof(struct res_get_bloque));

	switch (respuesta->header.id) {
	case GET_BLOQUE_ERROR:
		log_error_consola("No se pudo obtener el bloque %d", bloque_archivo);
		destroy_message(respuesta);
		return res;
		break;
	case GET_BLOQUE_OK:
		res->stream = malloc(respuesta->header.length);
		res->bloque_archivo = respuesta->argv[0];
		memcpy(res->stream, respuesta->stream, respuesta->header.length);
		log_info_interno("Mensaje get_bloque_ok recibido por el bloque: %d", bloque_archivo);
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
	struct arg_set_bloque* args = argumentos;
	log_info_interno("Empieza set_bloque del bloque %d", args->bloque_nodo);
	int res;
	int socket = args->socket;
	int size = strlen(args->chunk);

	t_msg* msg_solicitud = string_message(SET_BLOQUE, args->chunk, 2, args->bloque_nodo, size);
	free(args->chunk);
	res = enviar_mensaje(socket, msg_solicitud);
	free(args);
	if (res < 0) {
		log_error_consola("Mensaje set_bloque fallo por desconexion del nodo, socket: %d", socket);
		destroy_message(msg_solicitud);
		sem_post(&sem_set_bloque);
		return 1;
	}
	destroy_message(msg_solicitud);
	sem_post(&sem_set_bloque);
	t_msg* respuesta = recibir_mensaje(socket);
	if (respuesta == NULL) {
		log_error_consola("Mensaje set_bloque fallo porque el nodo no respondió OK, se asume desconexion del nodo, socket: %d", socket);
		return 1;
	}

	switch (respuesta->header.id) {
	case SET_BLOQUE_OK:
		log_info_interno("Terminó set_bloque OK del bloque: %d", args->bloque_nodo);
		return 0;
		break;
	default:
		log_error_consola("Respuesta Incorrecta del set_bloque: %d", args->bloque_nodo);
		return 1;
	}
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
	char* nombre_archivo_final = string_new();
	int valor, res;

	char** parametros = string_n_split(mensaje, 3, "|");
	free(mensaje);
	string_append(&nombre_archivo_tmp, parametros[0]);
	string_append(&nombre_nodo, parametros[1]);
	string_append(&nombre_archivo_final, parametros[2]);
	nodo = nodo_operativo_por_nombre(nombre_nodo);
	free(nombre_nodo);
	if (nodo == NULL) {
		log_error_interno("El nodo no se encuentra operativo para solicitarle un archivo temporal");
		free(nombre_archivo_tmp);
		return id_message(GET_ARCHIVO_TMP_ERROR);
	}

	t_msg* msg_solicitud = string_message(GET_FILE_CONTENT, nombre_archivo_tmp, 0);
	res = enviar_mensaje(nodo->socket, msg_solicitud);
	if (res < 0) {
		log_error_consola("Mensaje GET_FILE_CONTENT fallo por desconexion del nodo, socket: %d", nodo->socket);
		desconexion_nodo(nodo->socket);
		free(nombre_archivo_tmp);
		return id_message(GET_ARCHIVO_TMP_ERROR);
	}
	t_msg* msg_respuesta = recibir_mensaje(nodo->socket);
	if (msg_respuesta == NULL) {
		log_error_consola("Mensaje recibido despues del GET_FILE_CONTENT es NULL, se asume desconexion del nodo, socket: %d", nodo->socket);
		desconexion_nodo(nodo->socket);
		free(nombre_archivo_tmp);
		return id_message(GET_ARCHIVO_TMP_ERROR);
	}
	switch (msg_respuesta->header.id) {
	case GET_FILE_CONTENT_ERROR:
		log_error_consola("No se pudo obtener el archivo temporal del nodo");
		respuesta = id_message(GET_ARCHIVO_TMP_ERROR);
		break;
	case GET_FILE_CONTENT_OK:
		valor = copiar_archivo_temporal_a_mdfs(nombre_archivo_final, msg_respuesta->stream);
		if (valor) {
			log_error_consola("No se pudo copiar el archivo tmp al mdfs");
			respuesta = id_message(GET_ARCHIVO_TMP_ERROR);
		} else {
			log_info_consola("Se copió el archivo %s al mdfs", nombre_archivo_final);
			respuesta = id_message(GET_ARCHIVO_TMP_OK);
		}
		break;
	default:
		log_error_consola("Respuesta Incorrecta del nodo.");
		respuesta = id_message(GET_ARCHIVO_TMP_ERROR);
	}
	free(nombre_archivo_tmp);
	destroy_message(msg_respuesta);
	return respuesta;
}

