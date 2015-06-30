/*
 * filesystem.h
 *
 *  Created on: 23/5/2015
 *      Author: utnso
 */

#ifndef SRC_FILESYSTEM_H_
#define SRC_FILESYSTEM_H_

#include <db.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/log.h>
#include <stdio.h>
#include "directorio.h"
#include "archivo.h"
#include "nodo.h"
#include "database.h"
#include "server.h"
#include <utiles/log.h>

#define BLOQUE_SIZE_20MB 20*1024*1024

int inicializar_filesystem(bool formatea, int cantidad_nodos);
int eliminar_archivo(char* ruta_archivo);
int copiar_archivo_local_a_mdfs(char* ruta_local, char* ruta_mdfs);
int copiar_archivo_mdfs_a_local(char* ruta_local, char* ruta_mdfs);
int md5(char* ruta_mdfs);
int crear_directorio(char* ruta_padre, char* nombre);
int renombrar_archivo(char* ruta_archivo, char* nombre_nuevo);
int mover_archivo(char* ruta_archivo, char* ruta_padre_nuevo);
int eliminar_directorio(char* ruta_directorio);
int renombrar_directorio(char* ruta_directorio, char* nombre_nuevo);
int mover_directorio(char* ruta_directorio, char* ruta_padre_nuevo);
void crear_directorio_lista(t_directorio* directorio);
t_directorio* directorio_por_ruta(char* ruta);
t_directorio* directorio_por_id(db_recno_t id);
bool existe_directorio_con_nombre_en_directorio(char* nombre, t_directorio* directorio_padre);
bool existe_archivo_con_nombre_en_directorio(char* nombre, t_directorio* directorio_padre);
bool directorio_tiene_directorios_hijos(t_directorio* directorio);
bool directorio_tiene_archivos_hijos(t_directorio* padre);
t_archivo* archivo_por_ruta(char* ruta);
void free_puntero_puntero(char** lines);
t_list* directorios_hijos_de_directorio(t_directorio* directorio_actual);
t_list* archivos_hijos_de_directorio(t_directorio* directorio_actual);
t_directorio* directorio_raiz();
void crear_directorio_raiz();
int registrar_nodo(t_nodo* nodo);
int conexion_reconexion_nodo(t_nodo* nodo_existente, t_nodo* nodo);
int ver_bloque_de_archivo(int numero_bloque, char* nodo);
int borrar_bloque_de_nodo(int numero_bloque_archivo, char* ruta_archivo);
int copiar_bloque_de_nodo_a_nodo(int numero_bloque_nodo, char* nombre_nodo_origen, char* nombre_nodo_destino);
int bloque_disponible_de_nodo(t_nodo* nodo);
t_nodo* nodo_aceptado_por_nombre(char* nombre_nodo);
int cantidad_bloques_totales();
int cantidad_bloques_libres();
int proximos_nodos_disponibles(t_nodo* nodos[3]);
bool _nodo_conectado(t_nodo* nodo);
void list_add_directorio(t_directorio* directorio);
t_directorio* list_find_directorio(bool (*closure)(void*));
void list_add_archivo(t_archivo* archivo);
t_archivo* list_find_archivo(bool (*closure)(void*));
t_nodo* nodo_pendiente_por_nombre(char* nombre_nodo);
void list_add_nodos_aceptados(t_nodo* nodo);
void list_add_nodos_pendientes(t_nodo* nodo);
void list_add_nodos_operativos(t_nodo* nodo);
void list_iterate_nodos_pendientes(void (*closure)(void*));
void list_iterate_nodos_operativos(void (*closure)(void*));
void list_iterate_nodos_aceptados(void (*closure)(void*));
void list_iterate_archivos(void (*closure)(void*));
t_nodo* list_find_nodos_aceptados(bool (*closure)(void*));
t_nodo* list_find_nodos_operativos(bool (*closure)(void*));
t_nodo* list_find_nodos_pendientes(bool (*closure)(void*));
t_nodo* nodo_operativo_por_nombre(char* nombre_nodo);
int agregar_nodo(char* nombre_nodo);
int eliminar_nodo(char* nombre_nodo);
void desconectar_nodo(int socket);
void desconectar_marta(int socket);
void actualizar_disponibilidad_archivos_por_desconexion(t_nodo* nodo);
void actualizar_disponibilidad_archivos_por_reconexion(t_nodo* nodo);
void pasar_a_operativo();

#endif /* SRC_FILESYSTEM_H_ */
