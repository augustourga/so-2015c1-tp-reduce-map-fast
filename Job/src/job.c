/*
 * Job.c
 *
 *  Created on: 25/4/2015
 *      Author: utnso
 */
#include "job.h"

int cantidad_hilos = 0;
sem_t sem_sin_hilos;

pthread_mutex_t mutex_cantidad_hilos = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char* argv[]) {

	log_crear("DEBUG", LOG_FILE, PROCESO);

	obtenerConfiguracion(argv[1]);
	sem_init(&sem_sin_hilos, 0, 1);
	conectarseAMarta();

	esperarTareas();

	return 0;

}

void esperarTareas() {
	log_debug_consola("Se comienza a recibir solicitudes de tareas de map o reduce de MaRTA");

	fd_set read_fds;    // master file descriptor list
	int socket_actual;
	int fdmax;

	FD_ZERO(&read_fds);
	FD_SET(marta_sock, &read_fds);
	fdmax = marta_sock;

	while (true) {

		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			log_error_consola("Falló el select");
			perror("Falló el select. Error");
		}

		for (socket_actual = 0; socket_actual <= fdmax; socket_actual++) {
			if (FD_ISSET(socket_actual, &read_fds)) {
				if (socket_actual == marta_sock) {

					t_msg* mensaje_actual = recibir_mensaje(marta_sock);

					if (!mensaje_actual) {
						log_error_consola("Se perdió la conexión con MaRTA IP: %s - Puerto: %d", configuracion->ip_marta, configuracion->puerto_marta);
						exit(1);
					}

					log_debug_interno("Se recibió mensaje de MaRTA. Header.Id: %s - Argc: %d - Largo Stream: %d", id_string(mensaje_actual->header.id),
							mensaje_actual->header.argc, mensaje_actual->header.length);

					if (mensaje_actual->header.id == EJECUTAR_REDUCE) {
						t_params_hiloReduce* paramsR = (t_params_hiloReduce*) malloc(sizeof(t_params_hiloMap));
						char** argumentos;
						argumentos = string_split(mensaje_actual->stream, "|");
						paramsR->archivos_tmp = queue_create();
						strcpy(paramsR->ip, argumentos[0]);
						paramsR->nombre_nodo = string_duplicate(argumentos[1]);
						paramsR->puerto = mensaje_actual->argv[0];
						paramsR->id_operacion = mensaje_actual->argv[1];
						paramsR->archivo_final = argumentos[2];
						mensaje_actual = recibir_mensaje(marta_sock);

						if (!mensaje_actual) {
							log_error_consola("Se perdió la conexión con MaRTA IP: %s - Puerto: %d", configuracion->ip_marta, configuracion->puerto_marta);
							exit(1);
						}

						while (mensaje_actual->header.id != FIN_ENVIO_MENSAJE) {

							log_debug_interno("Se recibió mensaje de MaRTA. Header.Id: %s - Argc: %d - Largo Stream: %d", id_string(mensaje_actual->header.id),
									mensaje_actual->header.argc, mensaje_actual->header.length);

							queue_push(paramsR->archivos_tmp, (void*) mensaje_actual);
							mensaje_actual = recibir_mensaje(marta_sock);

							if (!mensaje_actual) {
								log_error_consola("Se perdió la conexión con MaRTA IP: %s - Puerto: %d", configuracion->ip_marta, configuracion->puerto_marta);
								exit(1);
							}
						}
						levantarHiloReduce(paramsR);
					}
					if (mensaje_actual->header.id == EJECUTAR_MAP) {
						t_params_hiloMap* params = (t_params_hiloMap*) malloc(sizeof(t_params_hiloMap));
						char** argumentos = string_split(mensaje_actual->stream, "|");
						strcpy(params->ip, argumentos[0]);
						params->nombre_nodo = string_duplicate(argumentos[1]);
						params->archivo_final = string_duplicate(argumentos[2]);
						params->puerto = mensaje_actual->argv[0];
						params->id_operacion = mensaje_actual->argv[1];
						params->bloque = mensaje_actual->argv[2];
						levantarHiloMapper(params);
						usleep(30000);
					}

					if (mensaje_actual->header.id == MDFS_NO_OPERATIVO) {
						log_info_consola("ERROR - El MDFS no esta operativo. Reintente mas tarde.");
						terminar_job();
					}
					if (mensaje_actual->header.id == INFO_ARCHIVO_ERROR) {
						log_info_consola("ERROR - No se encontro la informacion del archivo %s.Compruebe la existencia del archivo y vuelva a intentarlo.",
								mensaje_actual->stream);
						terminar_job();
					}
					if (mensaje_actual->header.id == FIN_MAP_ERROR) {
						log_info_consola("ERROR - No se pudo ejecutar map. Revise disponibilidad de los archivos y vuelva a ejecutar.");
						terminar_job();
					}
					if (mensaje_actual->header.id == FIN_REDUCE_ERROR) {
						log_info_consola("ERROR - No se pudo ejecutar reduce. Revise estado del MDFS y vuelva a ejecutar.");
						terminar_job();
					}
				}
			}
		}
	}
}

void obtenerConfiguracion(char* path) {
	t_config* config;

	configuracion = (t_Datos_configuracion*) malloc(sizeof(t_Datos_configuracion));
	config = config_create(path);

	if (config_has_property(config, "IP_MARTA")) {
		configuracion->ip_marta = strdup(config_get_string_value(config, "IP_MARTA"));
	} else {
		log_error_consola("El archivo de configuración %s debe tener un IP_MARTA", path);
		exit(1);
	}
	if (config_has_property(config, "REDUCE")) {
		configuracion->reduce = read_whole_file(config_get_string_value(config, "REDUCE"));
	} else {
		log_error_consola("El archivo de configuración %s debe tener un REDUCE", path);
		exit(1);
	}
	if (config_has_property(config, "MAPPER")) {
		configuracion->mapper = read_whole_file(config_get_string_value(config, "MAPPER"));
	} else {
		log_error_consola("El archivo de configuración %s debe tener un MAPPER", path);
		exit(1);
	}
	if (config_has_property(config, "PUERTO_MARTA")) {
		configuracion->puerto_marta = config_get_int_value(config, "PUERTO_MARTA");
	} else {
		log_error_consola("El archivo de configuración %s debe tener un PUERTO_MARTA", path);
		exit(1);
	}
	if (config_has_property(config, "COMBINER")) {
		configuracion->combiner = !strcmp(config_get_string_value(config, "COMBINER"), "SI") ? 1 : 0;
	} else {
		log_error_consola("El archivo de configuración %s debe tener un COMBINER", path);
		exit(1);
	}
	if (config_has_property(config, "RESULTADO")) {
		configuracion->resultado = strdup(config_get_string_value(config, "RESULTADO"));
	} else {
		log_error_consola("El archivo de configuración %s debe tener un RESULTADO", path);
		exit(1);
	}
	if (config_has_property(config, "ARCHIVOS")) {
		configuracion->archivos = strdup(config_get_string_value(config, "ARCHIVOS"));
	} else {
		log_error_consola("El archivo de configuración %s debe tener un ARCHIVOS", path);
		exit(1);
	}

	log_info_consola("El archivo de configuración %s fue cargado con éxito", path);

	config_destroy(config);
}

void conectarseAMarta() {
	marta_sock = client_socket(configuracion->ip_marta, configuracion->puerto_marta);

	if (marta_sock < 0) {
		log_error_consola("No se pudo conectar a MaRTA IP: %s - Puerto: %d", configuracion->ip_marta, configuracion->puerto_marta);
		exit(1);
	}

	log_debug_consola("Se conectó al proceso MaRTA IP: %s - Puerto: %d", configuracion->ip_marta, configuracion->puerto_marta);

	handshakeMarta();
}

int hiloReduce(void* dato) {
	sumar_hilo();
	t_msg* mensaje;
	t_msg* mensaje_respuesta;
	int ret;
	int i;
	t_params_hiloReduce* args = (t_params_hiloReduce*) dato;
	int nodo_sock = client_socket(args->ip, args->puerto);

	if (nodo_sock < 0) {
		log_error_consola("No se pudo conectar al proceso %s - IP: %s - Puerto: %d", args->nombre_nodo, args->ip, args->puerto);
		mensaje_respuesta = argv_message(FIN_REDUCE_ERROR, 1, args->id_operacion);
	} else {

		log_debug_consola("Se conectó al proceso %s - IP: %s - Puerto: %d", args->nombre_nodo, args->ip, args->puerto);

		mensaje = string_message(EJECUTAR_REDUCE, args->archivo_final, 1, args->id_operacion);

		log_debug_interno("Enviando mensaje de solicitud de reduce. Header.ID: %s - Argc: %d - Largo Stream: %d", id_string(mensaje->header.id),
				mensaje->header.argc, mensaje->header.length);

		ret = enviar_mensaje(nodo_sock, mensaje);

		mensaje = string_message(RUTINA, configuracion->reduce, 0);

		log_debug_interno("Enviando mensaje rutina. Header.ID: %s - Argc: %d - Largo Stream: %d", id_string(mensaje->header.id), mensaje->header.argc,
				mensaje->header.length);

		ret = enviar_mensaje(nodo_sock, mensaje);

		for (i = 0; i < args->archivos_tmp->elements->elements_count; i++) {
			mensaje = queue_pop(args->archivos_tmp);
			log_debug_interno("Enviando mensaje archivos de reduce. Header.ID: %s - Argc: %d - Largo Stream: %d", id_string(mensaje->header.id),
					mensaje->header.argc, mensaje->header.length);
			ret = enviar_mensaje(nodo_sock, mensaje);
		}

		mensaje = id_message(FIN_ENVIO_MENSAJE);

		log_debug_interno("Enviando mensaje fin de Mensaje. Header.ID: %s - Argc: %d - Largo Stream: %d", id_string(mensaje->header.id), mensaje->header.argc,
				mensaje->header.length);

		ret = enviar_mensaje(nodo_sock, mensaje);

		mensaje = recibir_mensaje(nodo_sock);

		if (!mensaje) { //Significa que recibir_mensaje devolvió NULL o sea que hubo un error en el recv o el nodo se desconectó
			mensaje_respuesta = argv_message(FIN_REDUCE_ERROR, 1, args->id_operacion);
		} else {
			mensaje_respuesta = argv_message(mensaje->header.id, 1, args->id_operacion);
			log_debug_interno("Se recibió mensaje de %s. Header.Id: %s - Argc: %d - Largo Stream: %d", args->nombre_nodo, id_string(mensaje->header.id),
					mensaje->header.argc, mensaje->header.length);
		}
	}

	//Se reenvía el resultado del reduce a marta
	log_debug_interno("Enviando mensaje respuesta a MaRTA. Header.ID: %s - Argc: %d - Largo Stream: %d", id_string(mensaje_respuesta->header.id),
			mensaje_respuesta->header.argc, mensaje_respuesta->header.length);

	ret = enviar_mensaje(marta_sock, mensaje_respuesta);
	if (ret != 0) {
		log_error_consola("fallo envio de mensaje");
	}
	destroy_message(mensaje_respuesta);
	destroy_message(mensaje);

	//TODO: No habría que cerrar la conexión con el nodo como en el HiloMap?
	//Lo agregue, no entiendo por que no deberia no hacerse
	shutdown(nodo_sock, 2);
	restar_hilo();
	return 0;
}

int hiloMap(void* dato) {
	sumar_hilo();
	t_msg* mensaje;
	t_msg* mensaje_respuesta;
	t_params_hiloMap* args = (t_params_hiloMap*) dato;
	int nodo_sock = client_socket(args->ip, args->puerto);
	int ret;

	if (nodo_sock < 0) {
		log_error_consola("No se pudo conectar al proceso %s - IP: %s - Puerto: %d", args->nombre_nodo, args->ip, args->puerto);
		mensaje_respuesta = argv_message(FIN_MAP_ERROR, 1, args->id_operacion);
	} else {

		mensaje = string_message(EJECUTAR_MAP, args->archivo_final, 1, args->bloque);

		log_debug_interno("Enviando mensaje de solicitud de reduce. Header.ID: %s - Argc: %d - Largo Stream: %d", id_string(mensaje->header.id),
				mensaje->header.argc, mensaje->header.length);

		ret = enviar_mensaje(nodo_sock, mensaje);

		mensaje = string_message(RUTINA, configuracion->mapper, 1, args->id_operacion);

		log_debug_interno("Enviando mensaje de rutina. Header.ID: %s - Argc: %d - Largo Stream: %d", id_string(mensaje->header.id), mensaje->header.argc,
				mensaje->header.length);

		ret = enviar_mensaje(nodo_sock, mensaje);

		mensaje = recibir_mensaje(nodo_sock);

		if (!mensaje) { //Significa que recibir_mensaje devolvió NULL o sea que hubo un error en el recv o el nodo se desconectó
			mensaje_respuesta = argv_message(FIN_MAP_ERROR, 1, args->id_operacion);
		} else {
			mensaje_respuesta = argv_message(mensaje->header.id, 1, args->id_operacion);
			log_debug_interno("Se recibió mensaje de %s. Header.Id: %s - Argc: %d - Largo Stream: %d", args->nombre_nodo, id_string(mensaje->header.id),
					mensaje->header.argc, mensaje->header.length);
		}
	}
	//Se reenvía el resultado del map a marta
	log_debug_interno("Enviando mensaje respuesta a MaRTA. Header.ID: %s - Argc: %d - Largo Stream: %d", id_string(mensaje_respuesta->header.id),
			mensaje_respuesta->header.argc, mensaje_respuesta->header.length);

	ret = enviar_mensaje(marta_sock, mensaje_respuesta);
	if (ret != 0) {
		log_error_consola("Fallo envio de Mensaje");
	}
	destroy_message(mensaje_respuesta);
	destroy_message(mensaje);

	//CERRAR CONEXIÓN CON EL NODO//
	shutdown(nodo_sock, 2);
	restar_hilo();
	return 0;
}

void handshakeMarta() {
	char* str_mensaje = string_new();
	int ret;
	string_append(&str_mensaje, configuracion->resultado);
	string_append(&str_mensaje, "|");
	string_append(&str_mensaje, configuracion->archivos);
	t_msg* mensaje = string_message(CONEXION_JOB, str_mensaje, 1, configuracion->combiner);

	log_debug_interno("Enviando mensaje de solicitud de inicio de Job a MaRTA. Header.ID: %s - Archivos: %s - Resultado: %s", id_string(CONEXION_JOB),
			configuracion->archivos, configuracion->resultado);

	ret = enviar_mensaje(marta_sock, mensaje);

	if (ret < 0) {
		log_error_consola("Falló el envío de mensaje de solicitud de inicio de Job");
		exit(1);
	}

	destroy_message(mensaje);
}

void levantarHiloMapper(t_params_hiloMap* nodo) {
	pthread_t thHiloMap;
	log_info_consola("Creando Hilo Mapper");
	int dato = pthread_create(&thHiloMap, NULL, (void *) hiloMap, (void*) nodo);
	if (dato != 0) {
		log_error_consola("El thread mapper no pudo ser creado.");
		exit(1);
	}
}

void levantarHiloReduce(t_params_hiloReduce* nodo) {
	pthread_t thHiloReduce;
	int dato = pthread_create(&thHiloReduce, NULL, (void *) hiloReduce, (void*) nodo);
	if (dato != 0) {
		log_error_consola("El thread reduce no pudo ser creado.");
		exit(1);
	}
}

void terminar_job() {

	log_info_interno("Esperando a que terminen los hilos pendientes");
	sem_wait(&sem_sin_hilos);
	log_info_interno("Hilos finalizados. Terminando proceso Job");
	exit(1);
}

void sumar_hilo() {

	pthread_mutex_lock(&mutex_cantidad_hilos);

	if (cantidad_hilos == 0) {
		sem_wait(&sem_sin_hilos);
	}
	cantidad_hilos += 1;
	pthread_mutex_unlock(&mutex_cantidad_hilos);

}

void restar_hilo() {
	pthread_mutex_lock(&mutex_cantidad_hilos);

	cantidad_hilos -= 1;
	if (cantidad_hilos == 0) {
		sem_post(&sem_sin_hilos);
	}
	pthread_mutex_unlock(&mutex_cantidad_hilos);

}

