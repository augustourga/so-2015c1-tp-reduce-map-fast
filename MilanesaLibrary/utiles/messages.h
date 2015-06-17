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
	/*************************JOB*******************************/
	EJECUTAR_MAP,
	EJECUTAR_REDUCE,
	FIN_MAP,
	FIN_REDUCE,
	CONEXION_JOB,
	JOB_MARTA_ARCH,
	ARCHIVO_JOB_MAP,
	ARCHIVO_JOB_REDUCE,
	FIN_ENVIO_ARCH,
	/***********************NODO******************************/
	NODO_MAP,
	NODO_REDUCE,
	SET_BLOQUE,
	GET_BLOQUE,
	GET_FILE_CONTENT,
	INFO_NODO

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
