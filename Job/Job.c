/*
 * Job.c
 *
 *  Created on: 25/4/2015
 *      Author: utnso
 */
#include "Job.h"


int main()
{

	Log_job = log_create(LOG_FILE,PROCESO,1,LOG_LEVEL_TRACE);
	if(obtenerConfiguracion())
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

	handshake_marta();

	while(1){
		mensaje_actual = recibir_mensaje(marta_sock);

		if(mensaje_actual->header.id == EJECUTAR_REDUCE)
		{
			//levantar_hilo_reduce(params);

		}
		if(mensaje_actual->header.id == EJECUTAR_MAP)
		{
			t_params_hiloMap* params =(t_params_hiloMap*)malloc(sizeof(t_params_hiloMap));
			memcpy(params->ip,mensaje_actual->stream,15);
			params->puerto = mensaje_actual->argv[0];

			destroy_message(mensaje_actual);
			mensaje_actual =recibir_mensaje(marta_sock);
			params->archivo= malloc(strlen(mensaje_actual->stream)+1);
			strcpy(params->archivo,mensaje_actual->stream);
			levantar_hilo_mapper(params);

		}
	}

	return 0;

}

int obtenerConfiguracion()
{
	t_config* config;

	configuracion = (t_Datos_configuracion*)malloc(sizeof(t_Datos_configuracion));
	config = config_create(CONFIG_PATH);

	configuracion->ip_marta= strdup(config_get_string_value(config, "IP_MARTA"));
	configuracion->reduce = read_whole_file(config_get_string_value(config, "REDUCE"));
	configuracion->mapper = read_whole_file(config_get_string_value(config, "MAPPER"));
	configuracion->puerto_marta = config_get_int_value(config,"PUERTO_MARTA");
	configuracion->combiner =strcmp(config_get_string_value(config,"COMBINER"),"SI")?1:0;
	configuracion->resultado = strdup(config_get_string_value(config, "RESULTADO"));
	configuracion->archivos = config_get_array_value(config, "ARCHIVOS");
	configuracion->cant_archivos = config_get_int_value(config, "CANT_ARCHIVOS");


	config_destroy(config);
	return 0;
}

void mostrarPorPantalla()
{
	int i;
	char* mensaje= malloc(sizeof(char*));
	sprintf(mensaje,"ip_marta:%s.",configuracion->ip_marta);
	log_trace(Log_job,mensaje);
	sprintf(mensaje,"puerto_marta:%d.",configuracion->puerto_marta);
	log_trace(Log_job,mensaje);
	sprintf(mensaje,"resultado:%s.",configuracion->resultado);
	log_trace(Log_job,mensaje);
	sprintf(mensaje,"combiner:%d.",configuracion->combiner);
	log_trace(Log_job,mensaje);
	for(i=0;i<2;i++)
	{
		sprintf(mensaje,"Archivo:%s.",configuracion->archivos[i]);
		log_trace(Log_job,mensaje);
	}

}

int conexionMaRTA()
{
	char buffer[MAXSIZE];
	//marta_sock = obtener_socket();
	//Falta validar que la conexión no falle, el Procedimiento solo imprime por pantalla
	//conectar_socket(configuracion->puerto_marta,configuracion->ip_marta,marta_sock);
	marta_sock = client_socket(configuracion->ip_marta, configuracion->puerto_marta);


	log_debug(Log_job,buffer);

	return 0;
}

int HiloReduce(void* dato)
{
	t_msg* mensaje;
	t_params_hiloMap* nodo =(t_params_hiloMap*)dato;

	int nodo_sock = client_socket(nodo->ip, nodo->puerto);

	mensaje = string_message(EJECUTAR_REDUCE,configuracion->reduce,0);
	printf("JOB-TAMAÑO DEL REDUCE : %d",strlen(configuracion->reduce));//DENBUG
	enviar_mensaje(nodo_sock,mensaje);
	mensaje = recibir_mensaje(nodo_sock);

	//Se reenvía el resultado del map a marta
	enviar_mensaje(marta_sock,mensaje);
	destroy_message(mensaje);
	return 0;
}

int HiloMap(void* dato)
{
	t_msg* mensaje;
	t_params_hiloMap* nodo =(t_params_hiloMap*)dato;
	int nodo_sock = client_socket(nodo->ip, nodo->puerto);

	mensaje = string_message(EJECUTAR_MAP,configuracion->mapper,0);
	enviar_mensaje(nodo_sock,mensaje);
	//ENVÍO ARCHIVO AL QUE SE APLICA EL MAP
	destroy_message(mensaje);
	//mensaje= string_message(ARCHIVO_JOB_MAP,nodo->archivo,0);

	enviar_mensaje(nodo_sock,mensaje);


	mensaje = recibir_mensaje(nodo_sock);

	//CERRAR CONEXIÓN CON EL NODO//

	//Se reenvía el resultado del map a marta
	enviar_mensaje(marta_sock,mensaje);
	destroy_message(mensaje);

	shutdown(nodo_sock,2);
	return 0;

	//ELIMINAR THREAD
}

int handshake_marta()
{

	int i;
	t_msg* mensaje = id_message(CONEXION_JOB);//,1,configuracion->archivos[0]);
 	enviar_mensaje(marta_sock,mensaje);
	for(i=0;i<configuracion->cant_archivos;i++)
	{
		//ENVÍO TODOS LOS ARCHIVOS
		destroy_message(mensaje);
		mensaje = string_message(CONEXION_JOB,configuracion->archivos[i],0);
		enviar_mensaje(marta_sock,mensaje);
	}
	//ENVÍO EL FIN DE ARCHIVOS
	destroy_message(mensaje);
	enviar_mensaje(marta_sock,id_message(FIN_ENVIO_MENSAJE));

	destroy_message(mensaje);
	mensaje = recibir_mensaje(marta_sock);
	if(mensaje->header.id==FIN_ENVIO_MENSAJE)
	{
		log_trace(Log_job,"Enviados los archivos a Marta con éxito.");
	}

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

int levantar_hilo_reduce(t_params_hiloMap* nodo)
{
	th_hilo_reduce = malloc(sizeof(pthread_t));
	int dato = pthread_create(th_hilo_reduce, NULL, (void *) HiloMap,
					(void*)nodo);
				if (dato != 0) {
					log_error(Log_job,
							"El thread reduce no pudo ser creado.");
					return EXIT_FAILURE;
				}
	return 0;
}

