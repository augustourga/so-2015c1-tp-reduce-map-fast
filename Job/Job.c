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
		if(mensaje_actual->header.id == EJECUTAR_REDUCE)
		{
			int dato = pthread_create(th_hilo_reduce, NULL, (void *) HiloReduce,
					mensaje_actual->argv);
			if (dato != 0) {
				log_error(Log_job,
						"El thread reducer no pudo ser creado.");
				return EXIT_FAILURE;
			}
		}
		if(mensaje_actual->header.id == EJECUTAR_MAP)
		{
			int dato = pthread_create(th_hilo_map, NULL, (void *) HiloMap,
				mensaje_actual->argv);
			if (dato != 0) {
				log_error(Log_job,
						"El thread reducer no pudo ser creado.");
				return EXIT_FAILURE;
			}
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
	configuracion->reduce = strdup(config_get_string_value(config, "REDUCE"));
	configuracion->mapper = strdup(config_get_string_value(config, "MAPPER"));
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

int HiloReduce(char* ip_nodo, int puerto_nodo)
{
	int nodo_sock;
	conectar_socket(puerto_nodo,ip_nodo,nodo_sock);

	t_msg* mensaje_fin_reduce = id_message(FIN_REDUCE);
	enviar_mensaje(marta_sock,mensaje_fin_reduce);
	destroy_message(mensaje_fin_reduce);

	return 0;
}

int HiloMap(char* ip_nodo, int puerto_nodo)
{
	int nodo_sock;
	conectar_socket(puerto_nodo,ip_nodo,nodo_sock);

	t_msg* mensaje_fin_map = id_message(FIN_MAP);
	enviar_mensaje(marta_sock,mensaje_fin_map);
	destroy_message(mensaje_fin_map);
	return 0;
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
