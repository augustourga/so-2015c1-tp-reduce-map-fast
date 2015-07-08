/*
 * Job.c
 *
 *  Created on: 25/4/2015
 *      Author: utnso
 */
#include "Job.h"


int main(int argc, char* argv[])
{
	//pthread_mutex_init(marta_mutex,NULL);
	Log_job = log_create(LOG_FILE,PROCESO,1,LOG_LEVEL_TRACE);
	if(obtenerConfiguracion(argv[0]))
	{
		log_error(Log_job,"Hubo errores en la carga de las configuraciones.");
	}
	//mostrarPorPantalla();// DEBUGGING

	if(!conexionMaRTA())
	{
		log_trace(Log_job,"Conexión con MaRTA realizada con éxito.");
	}
	else
	{
		log_error(Log_job,"Conexión con MaRTA finalizó con error.");
		return -12;
	}


	while(1){

		//pthread_mutex_lock(marta_mutex);
		mensaje_actual = recibir_mensaje(marta_sock);
		//pthread_mutex_unlock(marta_mutex);

		if(mensaje_actual->header.id == EJECUTAR_REDUCE)
		{
			t_params_hiloReduce* paramsR= (t_params_hiloReduce*)malloc(sizeof(t_params_hiloMap));
			char** argumentos;
			argumentos= string_split(mensaje_actual->stream,"|");
			paramsR->archivos_tmp = queue_create();
			strcpy(paramsR->ip,argumentos[0]);
			strcpy(paramsR->nombre_nodo,argumentos[1]);
			paramsR->puerto=mensaje_actual->argv[0];
			paramsR->id_operacion=mensaje_actual->argv[1];
			paramsR->archivo_final= argumentos[2];

			mensaje_actual=recibir_mensaje(marta_sock);
			while(mensaje_actual->header.id!=FIN_ENVIO_MENSAJE)
			{
				queue_push(paramsR->archivos_tmp,(void*) mensaje_actual);
				mensaje_actual= recibir_mensaje(marta_sock);
			}
			levantar_hilo_reduce(paramsR);

		}
		if(mensaje_actual->header.id == EJECUTAR_MAP)
		{
			t_params_hiloMap* params =(t_params_hiloMap*)malloc(sizeof(t_params_hiloMap));
			char** argumentos = string_split(mensaje_actual->stream,"|");
			strcpy(params->ip,argumentos[0]);
			params->archivo_final=argumentos[1];
			params->puerto = mensaje_actual->argv[0];
			params->id_operacion= mensaje_actual->argv[1];
			params->bloque= mensaje_actual->argv[2];
			levantar_hilo_mapper(params);

		}
	}

	return 0;

}

int obtenerConfiguracion(char* path)
{
	t_config* config;

	configuracion = (t_Datos_configuracion*)malloc(sizeof(t_Datos_configuracion));
	config = config_create(path);//CONFIG_PATH);

	configuracion->ip_marta= strdup(config_get_string_value(config, "IP_MARTA"));
 	configuracion->reduce = read_whole_file(config_get_string_value(config, "REDUCE"));
	configuracion->mapper = read_whole_file(config_get_string_value(config, "MAPPER"));
	configuracion->puerto_marta = config_get_int_value(config,"PUERTO_MARTA");
	configuracion->combiner =strcmp(config_get_string_value(config,"COMBINER"),"SI")?1:0;
	configuracion->resultado = strdup(config_get_string_value(config, "RESULTADO"));
	configuracion->archivos = strdup(config_get_string_value(config, "ARCHIVOS"));



	config_destroy(config);
	return 0;
}

int conexionMaRTA()
{
	char buffer[MAXSIZE];
	//marta_sock = obtener_socket();
	//Falta validar que la conexión no falle, el Procedimiento solo imprime por pantalla
	//conectar_socket(configuracion->puerto_marta,configuracion->ip_marta,marta_sock);
	marta_sock = client_socket(configuracion->ip_marta, configuracion->puerto_marta);


	log_debug(Log_job,buffer);

	handshake_marta();
	return 0;
}

int HiloReduce(void* dato)
{
	t_msg* mensaje;
	int i;
	t_params_hiloReduce* args = (t_params_hiloReduce*) dato;
	int nodo_sock = client_socket(args->ip, args->puerto);

	mensaje = string_message(EJECUTAR_REDUCE, args->archivo_final, 1,
			args->id_operacion);
	enviar_mensaje(nodo_sock, mensaje);

	mensaje = string_message(RUTINA, configuracion->reduce, 0);
	enviar_mensaje(nodo_sock, mensaje);
	for (i = 0; i < args->archivos_tmp->elements->elements_count; i++) {

		enviar_mensaje(nodo_sock, (t_msg*) queue_pop(args->archivos_tmp));
	}
	mensaje = id_message(FIN_ENVIO_MENSAJE);
	enviar_mensaje(nodo_sock, mensaje);
	mensaje = recibir_mensaje(nodo_sock);

	switch (mensaje->header.id) {
	case FIN_REDUCE_ERROR:
		mensaje = string_message(mensaje->header.id, args->nombre_nodo, 1,
				args->id_operacion);
		break;
	case FIN_REDUCE_OK:
		mensaje = argv_message(mensaje->header.id, 1, args->id_operacion);
		break;
	}



	//Se reenvía el resultado del reduce a marta
	//pthread_mutex_lock(marta_mutex);
	enviar_mensaje(marta_sock,mensaje);
	//pthread_mutex_unlock(marta_mutex);
	destroy_message(mensaje);
	return 0;
}

int HiloMap(void* dato)
{
	t_msg* mensaje;
	t_params_hiloMap* args =(t_params_hiloMap*)dato;
	int nodo_sock = client_socket(args->ip, args->puerto);

	mensaje = string_message(EJECUTAR_MAP,args->archivo_final,1,args->bloque);

	enviar_mensaje(nodo_sock,mensaje);
	destroy_message(mensaje);
	mensaje = string_message(RUTINA,configuracion->mapper,1,args->id_operacion);
	enviar_mensaje(nodo_sock,mensaje);
	destroy_message(mensaje);

	mensaje = recibir_mensaje(nodo_sock);
	mensaje= argv_message(mensaje->header.id,1,args->id_operacion);
	//Se reenvía el resultado del map a marta
	//pthread_mutex_lock(marta_mutex);
	enviar_mensaje(marta_sock,mensaje);
	//pthread_mutex_unlock(marta_mutex);
	destroy_message(mensaje);

	//CERRAR CONEXIÓN CON EL NODO//
	shutdown(nodo_sock,2);
	return 0;

	//ELIMINAR THREAD
}

int handshake_marta()
{
	char * str_mensaje;
	sprintf(str_mensaje,"%s|%s",configuracion->resultado,configuracion->archivos);
	t_msg* mensaje = string_message(CONEXION_JOB,str_mensaje,configuracion->combiner);
 	enviar_mensaje(marta_sock,mensaje);

	destroy_message(mensaje);
	mensaje = recibir_mensaje(marta_sock);

	return 0;
}

int levantar_hilo_mapper(t_params_hiloMap* nodo)
{
	th_hilo_map = malloc(sizeof(pthread_t));
	int dato = pthread_create(th_hilo_map, NULL, (void *) HiloMap,
					(void*)nodo);
				if (dato != 0) {
					log_error(Log_job,
							"El thread mapper no pudo ser creado.");
					return EXIT_FAILURE;
				}
				return 0;
}

int levantar_hilo_reduce(t_params_hiloReduce* nodo)
{
	th_hilo_reduce = malloc(sizeof(pthread_t));
	int dato = pthread_create(th_hilo_reduce, NULL, (void *) HiloReduce,
					(void*)nodo);
				if (dato != 0) {
					log_error(Log_job,
							"El thread reduce no pudo ser creado.");
					return EXIT_FAILURE;
				}
				return 0;
}

