/*
 * Job.c
 *
 *  Created on: 25/4/2015
 *      Author: utnso
 */
#include "Job.h"


int main()
{
	t_direccion_proceso* nodo =(t_direccion_proceso*)malloc(sizeof(t_direccion_proceso));
	Log_job = log_create(LOG_FILE,PROCESO,1,LOG_LEVEL_TRACE);
	if(obtenerConfiguracion())
	{
		log_error(Log_job,"Hubo errores en la carga de las configuraciones.");
	}
	mostrarPorPantalla();// DEBUGGING

	if(!conexionMaRTA())
	{
		log_trace(Log_job,"Conexión con MaRTA realizada con éxito.");
	}
	else
	{
		log_error(Log_job,"Conexión con MaRTA finalizó con error.");
		return -12;
	}

	enviar_mensaje_inicial_marta();

	while(1){
		mensaje_actual = recibir_mensaje(marta_sock);
		memcpy(nodo->ip,mensaje_actual->stream,15);
		nodo->puerto = mensaje_actual->argv[0];
		if(mensaje_actual->header.id == EJECUTAR_REDUCE)
		{
			levantar_hilo_reduce(nodo);

		}
		if(mensaje_actual->header.id == EJECUTAR_MAP)
		{
			levantar_hilo_mapper(nodo);

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
	marta_sock = obtener_socket();
	//Falta validar que la conexión no falle, el Procedimiento solo imprime por pantalla
	conectar_socket(configuracion->puerto_marta,configuracion->ip_marta,marta_sock);

	log_debug(Log_job,buffer);

	return 0;
}

int HiloReduce(void* dato)
{
	int nodo_sock = obtener_socket();
	t_msg* mensaje;
	t_direccion_proceso* nodo =(t_direccion_proceso*)dato;

	conectar_socket(nodo->puerto,nodo->ip,nodo_sock);
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
	int nodo_sock = obtener_socket();
	t_msg* mensaje;
	t_direccion_proceso* nodo =(t_direccion_proceso*)dato;

	conectar_socket(nodo->puerto,nodo->ip,nodo_sock);
	mensaje = string_message(EJECUTAR_MAP,configuracion->mapper,0);

	enviar_mensaje(nodo_sock,mensaje);

	int i;
	for(i=0;i<configuracion->cant_archivos;i++)
	{
		//ENVÍO TODOS LOS ARCHIVOS
		destroy_message(mensaje);
		mensaje = string_message(ARCHIVO_JOB_MAP,configuracion->archivos[i],0);
		enviar_mensaje(nodo_sock,mensaje);
	}
	//ENVÍO FIN DE ARCHIVOS
	destroy_message(mensaje);
	mensaje= id_message(FIN_ENVIO_ARCH);



	mensaje = recibir_mensaje(nodo_sock);

	//CERRAR CONEXIÓN CON EL NODO//

	//Se reenvía el resultado del map a marta
	enviar_mensaje(marta_sock,mensaje);
	destroy_message(mensaje);
	return 0;

	//ELIMINAR THREAD
}

int enviar_mensaje_inicial_marta()
{

//	int i;
	t_msg* mensaje_conexion_marta = id_message(CONEXION_JOB);//,1,configuracion->archivos[0]);
//	for(i=1;i<configuracion->cant_archivos;i++)
//	{
//		mensaje_conexion_marta = modify_message(CONEXION_JOB,mensaje_conexion_marta,i+1,configuracion->archivos[i]);
//
//	}

 	enviar_mensaje(marta_sock,mensaje_conexion_marta);
	return 0;
}

int levantar_hilo_mapper(t_direccion_proceso* nodo)
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

int levantar_hilo_reduce(t_direccion_proceso* nodo)
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

