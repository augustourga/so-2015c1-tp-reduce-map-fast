/*
 * Job.h
 *
 *  Created on: 25/4/2015
 *      Author: utnso
 */

#ifndef JOB_JOB_H_
#define JOB_JOB_H_

#include <stdio.h>

typedef struct
{
	char* ip_marta;
	int puerto_marta;
	char** archivos;
	char* resultado;
	int combiner; //1 indica SI, 0 indica NO
	FILE * mapper;
	FILE * reduce;
} Datos_configuracion;

typedef struct
{
	char rutina;//'M' para Mapper 'R' para Reducer
	char* ip_nodo;
	int puerto_nodo;
	char** datos;
	int cantidad_datos;

} Mensaje_Marta;

int obtenerConfiguracion();
int levantarArchivo(char*,FILE*);
void loguearLinea(char* linea,t_log_level);
void* hiloReduce(void*);
void* hiloMapper(void*);
int conexionMaRTA();
int conexionNodo(char*,int);
int ejecutarMaRTA();
void mostrarPorPantalla();
int operarConNodo();


#endif /* JOB_JOB_H_ */
