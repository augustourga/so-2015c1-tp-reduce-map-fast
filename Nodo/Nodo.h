/*
 * Nodo.h
 *
 *  Created on: 25/4/2015
 *      Author: utnso
 */

#ifndef NODO_NODO_H_
#define NODO_NODO_H_


#include <stdio.h>
#include <unistd.h>
#include <libio.h>
#include <string.h>
#include <stdlib.h>
#include <sockets.h>
#include <commons/config.h>
#define PATH "/home/utnso/workspace/NODO_CONFIG.config"

typedef struct{
	int sock_fs;
}t_conexion_nodo;

typedef struct{
	int sock_fs;
}t_bloque_content;

		 int PUERTO_FS;
		 char* IP_FS;
		 char* ARCHIVO_BIN;
		 char* DIR_TEMP;
		 char* NODO_NUEVO;
		 char* IP_NODO;
	     int PUERTO_NODO;


void levantarConfiguracionNodo();
void conectarFileSystem();
 getBloque(unNumero);

#endif /* NODO_NODO_H_ */
