/*
 * ejecuta_script.h
 *
 *  Created on: 1/5/2015
 *      Author: utnso
 */

#ifndef SRC_EJECUTA_SCRIPT_H_
#define SRC_EJECUTA_SCRIPT_H_

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/temporal.h>
#define BLOCK_SIZE_20MB 20971520

void error(char *s);

int ejecutar(char* path_entrada, char* path_ejecutable, char* path_salida);
char* generar_nombre_temporal( int mapreduce_id, char*map_o_reduce,int numeroBloque);
char* generar_nombre_rutina( char*map_o_reduce);
#endif /* SRC_EJECUTA_SCRIPT_H_ */
