/*
 * messages.h
 *
 *  Created on: 17/6/2015
 *      Author: utnso
 */

#ifndef UTILES_MESSAGES_H_
#define UTILES_MESSAGES_H_

#include "sockets.h"
#include <commons/string.h>

/****************** IDS DE MENSAJES. ******************/

typedef enum {
	NO_NEW_ID, // Valor centinela para evitar la modificación de id en modify_message()
	FIN_ENVIO_MENSAJE,
	/*************************JOB*****************************/
	//Funciones de map y reduce
	RUTINA,					//Stream de la rutina
	CONEXION_JOB,			//El job se conecta a MaRTA y le envía la tarea a ejecutar
							//archivo_final | archivo1 | archivo2 ..., combiner

	EJECUTAR_MAP,			//Marta a Job: Ip | nombre_arch_temp, puerto, Id_op (autogenerado), numero bloque
							//Job a Nodo nombre_arch_temp, numero_bloque
	FIN_MAP_OK,				//Id_op
	FIN_MAP_ERROR,			//Id_op

	EJECUTAR_REDUCE,		//Marta a Job: Ip | nombre_nodo | nombre_arch_temp , puerto, Id_op (autogenerado)
							//Job a Nodo nombre_arch_temp
	ARCHIVOS_NODO_REDUCE,	//Marta a Job: Ip | archivos1 ; archivo2 ..., puerto
	FIN_REDUCE_OK,			//Id_op
	FIN_REDUCE_ERROR,		//nombre_nodo ,Id_op

	/***********************NODO******************************/
	CONEXION_NODO,			//Nodo se conecta al MDFS y le pasa su info

	GET_BLOQUE,				//Numero bloque
	GET_BLOQUE_OK,
	GET_BLOQUE_ERROR,

	SET_BLOQUE,				//Numero bloque, datos
	SET_BLOQUE_OK,
	SET_BLOQUE_ERROR,

	GET_FILE_CONTENT,		//Nombre del archivo temporal
	GET_FILE_CONTENT_OK,
	GET_FILE_CONTENT_ERROR,

	GET_NEXT_ROW,			//Nombre de archivo temporal
	GET_NEXT_ROW_OK,
	GET_NEXT_ROW_ERROR,

	/***********************Filesystem***************************/
	CONEXION_MARTA,				// (Sin parametros)

	MDFS_NO_OPERATIVO,			//El mdfs no está operativo todavía

	INFO_ARCHIVO,				//Marta le pide al mdfs info sobre un archivo - Formato string_message: "{ruta_archivo}"
	INFO_ARCHIVO_OK,			//Mdfs devuelve la info - Formato string_message: "{nombre_nodo};{ip};{puerto};{bloque};(esto se repite por cada copia)|" (el | es solo cuando termina un bloque)
	INFO_ARCHIVO_ERROR,

	GET_ARCHIVO_TMP,			//Marta le pida al mdfs un archivo temporal de un nodo - Formato string_message: "{nombre_archivo_tmp}|{nombre_nodo}"
	GET_ARCHIVO_TMP_OK,
	GET_ARCHIVO_TMP_ERROR
} t_msg_id;

/****************** ESTRUCTURAS DE DATOS. ******************/

typedef struct {
	t_msg_id id;
	uint32_t length;
	uint16_t argc;
}__attribute__ ((__packed__)) t_header;

typedef struct {
	t_header header;
	char *stream;
	int32_t *argv;
}__attribute__ ((__packed__)) t_msg;

/****************** FUNCIONES T_MSG. ******************/

/*
 * Crea un t_msg sin argumentos, a partir del id.
 */
t_msg *id_message(t_msg_id id);

/*
 * Crea un t_msg a partir de count argumentos.
 */
t_msg *argv_message(t_msg_id id, uint16_t count, ...);

/*
 * Crea un t_msg a partir de un string y count argumentos.
 */
t_msg *string_message(t_msg_id id, char *message, uint16_t count, ...);

/*
 * Agrega nuevos argumentos a un mensaje (estilo FIFO).
 */
t_msg *modify_message(t_msg_id new_id, t_msg *old_msg, uint16_t new_count, ...);

/*
 * Elimina todos los argumentos existentes de un mensaje y agrega nuevos.
 */
t_msg *remake_message(t_msg_id new_id, t_msg *old_msg, uint16_t new_count, ...);

/*
 * Libera los contenidos de un t_msg.
 */
void destroy_message(t_msg *mgs);

/*
 * Recibe un t_msg a partir de un socket determinado.
 */
t_msg *recibir_mensaje(int sock_fd);

/*
 * Envia los contenidos de un t_msg a un socket determinado.
 */
int enviar_mensaje(int sock_fd, t_msg *msg);

/*
 * Muestra los contenidos y argumentos de un t_msg.
 */
void print_msg(t_msg *msg);

/*
 * Convierte t_msg_id a string.
 */
char *id_string(t_msg_id id);

#endif /* UTILES_MESSAGES_H_ */
