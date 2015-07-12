/*
 * files.h
 *
 *  Created on: 17/6/2015
 *      Author: utnso
 */

#ifndef UTILES_FILES_H_
#define UTILES_FILES_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <sys/types.h>
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE64_SOURCE 1
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "log.h"

#define handle_error(msj) \
	do{perror(msj);exit(EXIT_FAILURE);} while(0)

/****************** FUNCIONES FILE SYSTEM. ******************/

/*
 * Retorna un bool indicando si el archivo existe o no.
 */
bool file_exists(const char* filename);

/*
 * Retorna el tamaño de un archivo dado.
 */
size_t file_get_size(char* filename);

/*
 * Crea un archivo de size bytes de tamaño.
 */
void create_file(char *path, size_t size);

/*
 * Vacía el archivo indicado por path. Si no existe lo crea.
 */
void clean_file(char *path);

/*
 * Lee un archivo y retorna los primeros size bytes de su contenido.
 */
char* read_file(char *path, size_t size);

/*
 * Si existe, copia el contenido del archivo path en dest.
 */
void memcpy_from_file(char *dest, char *path, size_t size);

/*
 * Elimina los primeros size bytes del archivo path, y los retorna.
 */
char* read_file_and_clean(char *path, size_t size);

/*
 * Lee un archivo y retorna toodo su contenido.
 */
char* read_whole_file(char *path);

/*
 * Lee un archivo y retorna toodo su contenido, vaciándolo.
 */
char* read_whole_file_and_clean(char *path);

/*
 * Abre el archivo indicado por path (si no existe lo crea) y escribe size bytes de data.
 */
void write_file(char *path, char* data, size_t size);

char* file_combine(char* f1, char* f2);

/*
 * Devuelve el arhivo mappeado modo lectura y escritura
 */
void* file_get_mapped(char* filename);

void file_mmap_free(char* mapped, char* filename);

#endif /* UTILES_FILES_H_ */
