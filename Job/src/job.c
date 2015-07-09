/*
 * Job.c
 *
 *  Created on: 25/4/2015
 *      Author: utnso
 */
#include "job.h"

int main(int argc, char* argv[]) {

	log_crear("DEBUG", LOG_FILE, PROCESO);

	obtenerConfiguracion(argv[1]);

	conectarseAMarta();

	esperarTareas();

	return 0;

}

void esperarTareas() {
	log_debug_consola("Se comienza a recibir solicitudes de tareas de map o reduce de MaRTA");

	while (true) {
		t_msg* mensaje_actual = recibir_mensaje(marta_sock);

		log_debug_interno("Se recibió mensaje de MaRTA. Header.Id: %s - Argc: %d - Largo Stream: %d", id_string(mensaje_actual->header.id),
				mensaje_actual->header.argc, mensaje_actual->header.length);

		if (mensaje_actual->header.id == EJECUTAR_REDUCE) {
			t_params_hiloReduce* paramsR = (t_params_hiloReduce*) malloc(sizeof(t_params_hiloMap));
			char** argumentos;
			argumentos = string_split(mensaje_actual->stream, "|");
			paramsR->archivos_tmp = queue_create();
			strcpy(paramsR->ip, argumentos[0]);
			strcpy(paramsR->nombre_nodo, argumentos[1]);
			paramsR->puerto = mensaje_actual->argv[0];
			paramsR->id_operacion = mensaje_actual->argv[1];
			paramsR->archivo_final = argumentos[2];
			mensaje_actual = recibir_mensaje(marta_sock);
			while (mensaje_actual->header.id != FIN_ENVIO_MENSAJE) {

				log_debug_interno("Se recibió mensaje de MaRTA. Header.Id: %s - Argc: %d - Largo Stream: %d", id_string(mensaje_actual->header.id),
						mensaje_actual->header.argc, mensaje_actual->header.length);

				queue_push(paramsR->archivos_tmp, (void*) mensaje_actual);
				mensaje_actual = recibir_mensaje(marta_sock);
			}
			levantarHiloReduce(paramsR);
		}
		if (mensaje_actual->header.id == EJECUTAR_MAP) {
			t_params_hiloMap* params = (t_params_hiloMap*) malloc(sizeof(t_params_hiloMap));
			char** argumentos = string_split(mensaje_actual->stream, "|");
			strcpy(params->ip, argumentos[0]);
			params->archivo_final = argumentos[1];
			params->puerto = mensaje_actual->argv[0];
			params->id_operacion = mensaje_actual->argv[1];
			params->bloque = mensaje_actual->argv[2];
			levantarHiloMapper(params);
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
		configuracion->combiner = strcmp(config_get_string_value(config, "COMBINER"), "SI") ? 1 : 0;
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
	t_msg* mensaje;
	t_msg* mensaje_respuesta;
	int i;
	t_params_hiloReduce* args = (t_params_hiloReduce*) dato;
	int nodo_sock = client_socket(args->ip, args->puerto);

	mensaje = string_message(EJECUTAR_REDUCE, args->archivo_final, 1, args->id_operacion);
	enviar_mensaje(nodo_sock, mensaje);

	mensaje = string_message(RUTINA, configuracion->reduce, 0);
	enviar_mensaje(nodo_sock, mensaje);

	for (i = 0; i < args->archivos_tmp->elements->elements_count; i++) {
		enviar_mensaje(nodo_sock, (t_msg*) queue_pop(args->archivos_tmp));
	}

	mensaje = id_message(FIN_ENVIO_MENSAJE);
	enviar_mensaje(nodo_sock, mensaje);

	mensaje = recibir_mensaje(nodo_sock);

	if (!mensaje) { //Significa que recibir_mensaje devolvió NULL o sea que hubo un error en el recv o el nodo se desconectó
		mensaje_respuesta = argv_message(FIN_REDUCE_ERROR, 1, args->id_operacion);
	} else {
		switch (mensaje->header.id) {
		case FIN_REDUCE_ERROR:
			mensaje_respuesta = string_message(mensaje->header.id, args->nombre_nodo, 1, args->id_operacion);
			break;
		case FIN_REDUCE_OK:
			mensaje_respuesta = argv_message(mensaje->header.id, 1, args->id_operacion);
			break;
		default:
			log_error_consola("Mensaje Incorrecto: ", mensaje->header.id);
			break;
		}
	}

	//Se reenvía el resultado del reduce a marta
	enviar_mensaje(marta_sock, mensaje_respuesta);

	destroy_message(mensaje_respuesta);
	destroy_message(mensaje);

	//TODO: No habría que cerrar la conexión con el nodo como en el HiloMap?
	return 0;
}

int hiloMap(void* dato) {
	t_msg* mensaje;
	t_msg* mensaje_respuesta;
	t_params_hiloMap* args = (t_params_hiloMap*) dato;
	int ret;
	int nodo_sock = client_socket(args->ip, args->puerto);

	mensaje = string_message(EJECUTAR_MAP, args->archivo_final, 1, args->bloque);
	ret = enviar_mensaje(nodo_sock, mensaje);

	mensaje = string_message(RUTINA, configuracion->mapper, 1, args->id_operacion);
	ret = enviar_mensaje(nodo_sock, mensaje);

	mensaje = recibir_mensaje(nodo_sock);

	if (!mensaje) { //Significa que recibir_mensaje devolvió NULL o sea que hubo un error en el recv o el nodo se desconectó
		mensaje_respuesta = argv_message(FIN_MAP_ERROR, 1, args->id_operacion);
	} else {
		mensaje_respuesta = argv_message(mensaje->header.id, 1, args->id_operacion);
	}

	//Se reenvía el resultado del map a marta
	enviar_mensaje(marta_sock, mensaje_respuesta);

	destroy_message(mensaje_respuesta);
	destroy_message(mensaje);

	//CERRAR CONEXIÓN CON EL NODO//
	shutdown(nodo_sock, 2);
	return 0;
}

void handshakeMarta() {
	char* str_mensaje;
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

