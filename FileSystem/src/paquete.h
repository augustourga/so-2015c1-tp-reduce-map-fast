/*
 * paquete.h
 *
 *  Created on: 12/6/2015
 *      Author: utnso
 */

#ifndef PAQUETE_H_
#define PAQUETE_H_

#include <stdlib.h>
#include <string.h>

typedef enum op_type {
	//MDFS - MARTA
	MARTA_HANDSHAKE,					//Marta se identifica con el Mdfs cuando se conecta
	INFO_ARCHIVO,						//Marta le pide al mdfs info sobre un archivo
	INFO_ARCHIVO_OK,					//Mdfs devuelve la info
	INFO_ARCHIVO_FAIL,
	COPIAR_ARCHIVO,						//Marta le pide al mdfs que copie un archivo temporal de un nodo al mdfs
	COPIAR_ARCHIVO_OK,
	COPIAR_ARCHIVO_FAIL,

	//MDFS - NODO - NODO
	NODO_HANDSHAKE,						//Un Nodo se identifica con el mdfs o con otro nodo cuando se conecta
	GET_BLOQUE,							//Mdfs u otro nodo le pide a un nodo el stream de un bloque
	GET_BLOQUE_OK,						//El nodo devuelve el stream del bloque
	GET_BLOQUE_FAIL,
	SET_BLOQUE,							//Mdfs u otro nodo le manda al nodo un stream para que copie
	SET_BLOQUE_OK,
	SET_BLOQUE_FAIL,
	GET_TEMP_FILE,						//Mdfs u otro nodo le pide al nodo un archivo temporal
	GET_TEMP_FILE_OK,					//El nodo devuelve el stream del archivo temporal
	GET_TEMP_FILE_FAIL,

	//JOB - MARTA
	JOB_HANDSHAKE,						//Un Job se identifica con Marta cuando se conecta
	EXECUTE_JOB,						//Un Job le pide a Marta ejecutar una tarea de mapreduce
	EXECUTE_JOB_OK,
	EXECUTE_JOB_FAIL,
	MAP,								//Marta le indica al job que aplique una rutina de map
	MAP_OK,
	MAP_FAIL,
	REDUCE,								//Marta le indica el job que aplique una rutina de reduce
	REDUCE_OK,
	REDUCE_FAIL,

	//MAPPER - NODO
	MAPPER_HANDSHAKE,					//Un hilo mapper se identifica con el nodo y le indica que ejecute el map
	MAPPER_OK,
	MAPPER_FAIL,

	//REDUCER - NODO
	REDUCER_HANDSHAKE,					//Un hilo reducer se identifica con el nodo y le indica que ejecute el reduce
	REDUCER_OK,
	REDUCER_FAIL,

} t_op;

typedef struct {
	t_op cod_op;
	int largo_data;
	char* data;
} t_paquete;

char* paquete_serializar_mensaje(t_paquete* paquete, int* largo_paquete);
t_paquete* paquete_deserializar_mensaje(char* mensaje);
void paquete_serializar(void* to, void* from, int size, int* offset);
void paquete_deserializar(void* to, void* from, int size, int* offset);

#endif /* PAQUETE_H_ */
