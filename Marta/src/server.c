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
		log_debug_consola("Esperando nuevo mensaje");
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			log_error_consola("Falló el select");
			perror("Falló el select. Error");
			exit(3);
		}
		log_debug_consola("Nuevo mensaje recibido. procesandolo..");
		// run through the existing connections looking for data to read
		for (socket_actual = 0; socket_actual <= fdmax; socket_actual++) {
			if (FD_ISSET(socket_actual, &read_fds)) { // we got one!!
				if (socket_actual == listen_socket) {
					log_info_consola("Nueva conexion entrante");
					// handle new connections
					addrlen = sizeof addr_client;
					newfd = accept(listen_socket, (struct sockaddr *) &addr_client, &addrlen);
					if (newfd == -1) {
						log_error_consola("Falló el accept");
						perror("Falló el accept. Error");
					} else {
						log_debug_consola("Conexion nueva aceptada.");
						if (newfd > fdmax) {    // keep track of the max
							fdmax = newfd;
						}
						t_msg* mensaje = recibir_mensaje(newfd);
						if (mensaje->header.id == CONEXION_JOB) {
							log_info_consola("Creando nuevo hilo Job. job_id=%d", newfd);
							FD_SET(newfd, &master); // add to master set
							crear_hilo_job(newfd, mensaje->stream, mensaje->argv[0]);
						}
					}
				} else if (!socket_conectado(socket_actual)) {
					log_info_consola("Desconectando socket: %d", socket_actual);
					//manejar_desconexion(socket_actual); //TODO: Manejardesconexion
					FD_CLR(socket_actual, &master);
				} else {
					log_info_consola("mensaje del socket: %d", socket_actual);
					t_msg* mensaje = recibir_mensaje(socket_actual);
					if (mensaje == NULL) {
						log_info_consola("Mensaje en NULL.");
					} else {
						decodificar_mensaje(mensaje, socket_actual);
					}

				}
			}
		}
	}
}

void decodificar_mensaje(t_msg* mensaje, int socket) {

	switch (mensaje->header.id) {
	case FIN_MAP_OK:
		log_info_consola("FIN DE MAP OK. Socket: %d", socket);
		actualiza_job_map_ok(mensaje->argv[0], socket);
		break;
	case FIN_MAP_ERROR:
		log_info_consola("ERROR EN MAP. Socket: %d", socket);
		actualiza_job_map_error(mensaje->argv[0], socket);
		break;
	case FIN_REDUCE_OK:
		log_info_consola("FIN DE REDUCE OK. Socket: %d", socket);
		actualiza_job_reduce_ok(mensaje->argv[0], socket);
		break;
	case FIN_REDUCE_ERROR:
		log_info_consola("ERROR EN REDUCE. Socket: %d", socket);
		actualizar_job_reduce_error(mensaje->argv[0], socket, mensaje->stream);
		break;
	default:
		log_error_interno("Mensaje Incorrecto");
		break;
	}
}

void conectarse_a_mdfs(char* ip_mdfs, uint16_t puerto_mdfs) {

	log_debug_consola("Conectando al MDFS.");
	if ((socket_mdfs = client_socket(ip_mdfs, puerto_mdfs)) < 0) {
		log_error_consola("Error al conectarse a MDFS");
		exit(1);
	}

	t_msg* mensaje = id_message(CONEXION_MARTA);
	enviar_mensaje(socket_mdfs, mensaje);

	destroy_message(mensaje);
	log_debug_consola("Conexion con MFDS OK.");
}

char* get_info_archivo(t_job* job, char* ruta_mdfs) {

	char* ret = NULL;
	t_msg_id msg_id_job;
	bool mensaje_job = false;
	log_info_interno("Se busca la info del MDFS para el arhivo: %s", ruta_mdfs);
	t_msg* message = string_message(INFO_ARCHIVO, ruta_mdfs, 0);
	enviar_mensaje(socket_mdfs, message);
	t_msg* respuesta = recibir_mensaje(socket_mdfs);

	if (respuesta == NULL) {
		log_error_consola("Error al obtener la informacion del archivo: %s", ruta_mdfs);
		msg_id_job = INFO_ARCHIVO_ERROR;
		mensaje_job = true;
	} else if (respuesta->header.id == INFO_ARCHIVO_OK) {
		ret = string_duplicate(respuesta->stream);
		log_info_interno("Info obtenida: %s. arhivo: %s", &ret, ruta_mdfs);
	} else if (respuesta->header.id == INFO_ARCHIVO_ERROR) {
		log_error_consola("Error al obtener la informacion del archivo: %s. cancelando job.", ruta_mdfs);
		msg_id_job = INFO_ARCHIVO_ERROR;
		mensaje_job = true;
	} else if (respuesta->header.id == MDFS_NO_OPERATIVO) {
		log_error_consola("Filesystem no operativo. cancelando Job.");
		msg_id_job = MDFS_NO_OPERATIVO;
		mensaje_job = true;
	} else {
		log_error_consola("Error desconocido al llamar al MDFS. Archivo: %s. cancelando job.", ruta_mdfs);
		msg_id_job = INFO_ARCHIVO_ERROR;
		mensaje_job = true;
	}

	if (mensaje_job) {
		log_info_interno("Se envia mensaje a el job para que finalice.");
		t_msg* messageJob = string_message(msg_id_job, ruta_mdfs, 0);
		enviar_mensaje(job->socket, messageJob);
		destroy_message(messageJob);
	}

	destroy_message(respuesta);
	destroy_message(message);
	return ret;
}

void copiar_archivo_final(t_job* job) {
	log_info_consola("Copiando archivo final. Temporal: %s Nombre Final: %s", job->reduce_final->arch_tmp.nodo.nombre, job->archivo_final);
	char* stream = string_duplicate(job->archivo_final);
	string_append(&stream, "|");
	string_append(&stream, job->reduce_final->arch_tmp.nodo.nombre);

	t_msg* message = string_message(GET_ARCHIVO_TMP, stream, 0);
	enviar_mensaje(socket_mdfs, message);

}

void crear_hilo_job(int nuevo_job, char* stream, bool combiner) {
	struct arg_job* args = malloc(sizeof(struct arg_job));

	pthread_t hilo_job;

	args->stream = string_duplicate(stream);
	args->socket = nuevo_job;
	args->combiner = combiner;

	pthread_create(&hilo_job, NULL, (void*) procesa_job, (void*) args);
}

