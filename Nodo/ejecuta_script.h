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
#define BLOCK_SIZE_20MB 20971520

void error(char *s);

void ejecutar(char* path_entrada, char* path_ejecutable, char* path_salida);

#endif /* SRC_EJECUTA_SCRIPT_H_ */
