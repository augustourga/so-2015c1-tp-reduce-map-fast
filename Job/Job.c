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
		log_trace(Log_job,"Conexión con Marta realizada con éxito.");
	}
	else
	{
		log_error(Log_job,"Conexión con MaRTA finalizó con error.");
		//return -12;
	}


	return 0;

}

int obtenerConfiguracion()
{
	t_config* config;

	configuracion = (t_Datos_configuracion*)malloc(sizeof(t_Datos_configuracion));
	config = config_create(CONFIG_PATH);

	configuracion->ip_marta= config_get_string_value(config,"IP_MARTA");
	configuracion->puerto_marta = config_get_int_value(config,"PUERTO_MARTA");
	configuracion->combiner =strcmp(config_get_string_value(config,"COMBINER"),"SI")?1:0;
	configuracion->resultado = config_get_string_value(config,"RESULTADO");
	configuracion->archivos = config_get_array_value(config, "ARCHIVOS");


	if(levantarArchivo(config_get_string_value(config,"MAPPER"),configuracion->map))
	{
		log_error(Log_job,"Error al cargar el archivo con la rutina Mapper.");
		return -1;
	}

	if(levantarArchivo(config_get_string_value(config,"REDUCE"),configuracion->reduce))
	{
		log_error(Log_job,"Error al cargar el archivo con la rutina Reducer.");
		return -2;
	}
	config_destroy(config);
	return 0;
}

int levantarArchivo(char* direccion,t_Datos_archivo* puntero)
{
	puntero->stream = fopen(direccion,"rb");
	if(puntero==NULL)
	{
		return -1;
	}
	else
	{
		return fstat(fileno(puntero->stream),puntero->fstat);
	}
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
