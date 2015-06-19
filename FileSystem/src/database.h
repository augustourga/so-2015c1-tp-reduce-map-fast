/*
 * database.h
 *
 *  Created on: 23/5/2015
 *      Author: utnso
 */

#ifndef SRC_DATABASE_H_
#define SRC_DATABASE_H_

#include <db.h>
#include "directorio.h"
#include "archivo.h"
#include "filesystem.h"
#define	DIRECTORIOS_DB "files/directorios.db"
#define	ARCHIVOS_DB "files/archivos.db"
#define	NODOS_DB "files/nodos.db"

int formatear_filesystem() ;
int crear_database();
int recuperar_directorios();
int recuperar_archivos();
int recuperar_nodos();
int insertar_directorio(t_directorio* directorio);
int insertar_archivo(t_archivo* archivo);
int borrar_directorio(db_recno_t id);
int borrar_archivo(db_recno_t id);
int actualizar_directorio(t_directorio* directorio);
int actualizar_archivo(t_archivo* archivo);
int cerrar_database();
int obtener_directorio(db_recno_t id);
db_recno_t recno_archivos(bool formatea);
db_recno_t recno_directorios(bool formatea);
int insertar_nodo(t_nodo* nodo);

#endif /* SRC_DATABASE_H_ */
