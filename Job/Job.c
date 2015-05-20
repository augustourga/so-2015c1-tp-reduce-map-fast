/*
 * Job.c
 *
 *  Created on: 25/4/2015
 *      Author: utnso
 */
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sockets.h>
#include <commons/log.h>
#include <pthread.h>
#include "Job.h"
#include <commons/config.h>
#include <commons/collections/dictionary.h>

/*********Constantes*****/
#define CONFIG_PATH "/home/utnso/git/tp-2015-1c-milanesa/Job/Job.config"
#define LOG_FILE "/home/utnso/Job_log.txt"
#define PROCESO "Job"
#define MAXSIZE 1024
/************************/

/*********Variables globales*/
Datos_configuracion* configuracion;
Mensaje_Marta* mensaje_actual;
pthread_t* t_reduce;
pthread_t* t_mapper;
int marta_sock;
t_log* Log_job;

int pipePruebas[2];
/****************************/

//Nombre de instancia del Job se obtiene por parámetro//

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

	ejecutarMaRTA();

	return 0;

}

int obtenerConfiguracion()
{
	t_config* config;

	configuracion = (Datos_configuracion*)malloc(sizeof(Datos_configuracion));
	config = config_create(CONFIG_PATH);

	configuracion->ip_marta= config_get_string_value(config,"IP_MARTA");
	configuracion->puerto_marta = config_get_int_value(config,"PUERTO_MARTA");
	configuracion->combiner =strcmp(config_get_string_value(config,"COMBINER"),"SI")?1:0;
	configuracion->resultado = config_get_string_value(config,"RESULTADO");
	configuracion->archivos = config_get_array_value(config, "ARCHIVOS");


	if(levantarArchivo(config_get_string_value(config,"MAPPER"),configuracion->mapper))
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

int levantarArchivo(char* direccion,FILE* puntero)
{
	puntero = fopen(direccion,"rb");
	if(puntero==NULL)
	{
		return -1;
	}
	else return 0;
}

void *hiloReduce(void* ptr)
{
	return 1;
}

void *hiloMapper(void* ptr)
{
	return 1;
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

int conexionNodo(char* ip_nodo,int puerto_nodo)
{
	int sock= obtener_socket();
	conectar_socket(puerto_nodo,ip_nodo,sock);
	return 1;//Falta implementar
}

int ejecutarMaRTA()
{
	char* buffer= malloc(sizeof(Mensaje_Marta));
	while(1)
	{
		if	(recv(marta_sock,buffer,sizeof(Mensaje_Marta),0)!=-1)
		{
			mensaje_actual = (Mensaje_Marta*)&buffer;
		}

		operarConNodo();


	}
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

int operarConNodo()
{
	int sock_nodo=conexionNodo(mensaje_actual->ip_nodo,mensaje_actual->puerto_nodo);
	if(sock_nodo==-1) //COnexionNodo retornará -1 en caso de error y si no el socket
	{
		return -13;
	}
	if(mensaje_actual->rutina=='M')
	{
		log_trace(Log_job,"Se levantó hilo Mapper.");
		hiloMapper(t_mapper);
		return 0;
	}
	else if(mensaje_actual->rutina=='R')
	{
		log_trace(Log_job,"Se levantó hilo Reduce.");
		hiloReduce(t_reduce);
		return 0;
	}

	log_trace(Log_job,"Se envió un mensaje diferente a M o R.");
	return -1;

}


