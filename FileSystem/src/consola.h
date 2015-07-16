/*
 * consola.h
 *
 *  Created on: 23/5/2015
 *      Author: utnso
 */

#ifndef SRC_CONSOLA_H_
#define SRC_CONSOLA_H_

#include "directorio.h"
#include "archivo.h"
#include "filesystem.h"
#include "database.h"
#include <commons/string.h>
#include <stdio.h>
#include <utiles/log.h>

#define COMANDO_AYUDA "ayuda"
#define COMANDO_SALIR "salir"

#define COMANDO_FORMATEAR "formatear"
#define COMANDO_FORMATEAR_NODOS "formatear_nodos"

#define COMANDO_ELIMINAR_ARCHIVO "rm_archivo"
#define COMANDO_RENOMBRAR_ARCHIVO "cn_archivo"
#define COMANDO_MOVER_ARCHIVO "mv_archivo"

#define COMANDO_CREAR_DIRECTORIO "mk_directorio"
#define COMANDO_ELIMINAR_DIRECTORIO "rm_directorio"
#define COMANDO_RENOMBRAR_DIRECTORIO "cn_directorio"
#define COMANDO_MOVER_DIRECTORIO "mv_directorio"

#define COMANDO_COPIAR_MDFS_LOCAL "cp_mdfs_local"
#define COMANDO_COPIAR_LOCAL_MDFS "cp_local_mdfs"

#define COMANDO_MD5 "md5"

#define COMANDO_VER_BLOQUE "ls_bloque_archivo"
#define COMANDO_BORRAR_BLOQUE "rm_bloque"
#define COMANDO_COPIAR_BLOQUE "cp_bloque"

#define COMANDO_AGREGAR_NODO "ag_nodo"
#define COMANDO_ELIMINAR_NODO "rm_nodo"

#define COMANDO_LISTAR "ls"
#define COMANDO_LISTAR_NODOS "ls_nodo"
#define COMANDO_LISTAR_ARCHIVO "ls_archivo"
#define COMANDO_LISTAR_BLOQUES "ls_bloques"

#define COMANDO_DT "dt"
#define COMANDO_DF "df"

char* comando_preparado(char* comando);
void iniciar_consola();
int ejecutar_comando(char* comando);
int mostrar_ayuda();
void remueve_salto_de_linea(char* salida, char* texto);
void imprimir_ruta_completa_de_directorio(t_directorio * directorio);
int listar_hijos(char* ruta_directorio);
int listar_nodos();
int listar_nodos_operativos();
int espacio_total();
int espacio_libre();
void log_error_parametros_faltantes();

#endif /* SRC_CONSOLA_H_ */
