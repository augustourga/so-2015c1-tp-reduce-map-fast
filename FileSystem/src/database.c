#include "database.h"

DB *db_directorios;
DB *db_archivos;
DB *db_nodos;

pthread_mutex_t mutex_recno_directorios = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_recno_archivos = PTHREAD_MUTEX_INITIALIZER;

int formatear_filesystem() {
	remove(DIRECTORIOS_DB);
	remove(ARCHIVOS_DB);
	vaciar_bloques_nodos();

	inicializar_filesystem(true, 0);
	return 0;
}

int formatear_nodos() {
	remove(NODOS_DB);
	formatear_listas_nodos();
	crear_database_nodos();
	return 0;
}

int crear_database() {
	int ret;

	if ((ret = db_create(&db_directorios, NULL, 0)) != 0) {
		log_error_consola("db_create: %s", db_strerror(ret));
		return 1;
	}

	if ((ret = db_directorios->open(db_directorios,
	NULL, DIRECTORIOS_DB, NULL, DB_RECNO, DB_CREATE, 0664)) != 0) {
		db_directorios->err(db_directorios, ret, "%s", DIRECTORIOS_DB);
	}

	if ((ret = db_create(&db_archivos, NULL, 0)) != 0) {
		log_error_consola("db_create: %s", db_strerror(ret));
		return 1;
	}

	if ((ret = db_archivos->open(db_archivos,
	NULL, ARCHIVOS_DB, NULL, DB_RECNO, DB_CREATE, 0664)) != 0) {
		db_archivos->err(db_archivos, ret, "%s", ARCHIVOS_DB);
	}

	return 0;
}

int crear_database_nodos() {
	int ret;

	if ((ret = db_create(&db_nodos, NULL, 0)) != 0) {
		log_error_consola("db_create: %s", db_strerror(ret));
		return 1;
	}

	if ((ret = db_nodos->open(db_nodos,
	NULL, NODOS_DB, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
		db_nodos->err(db_nodos, ret, "%s", NODOS_DB);
	}

	return 0;
}

int recuperar_directorios() {
	DBC *cursor_directorios;
	DBT key, data;
	int cantidad_registros = 1; //Inicializado en 1 por el directorio raiz que no se persiste pero se crea
	int ret;

	/* Acquire a cursor for the database. */
	if ((ret = db_directorios->cursor(db_directorios, NULL, &cursor_directorios, 0)) != 0) {
		db_directorios->err(db_directorios, ret, "DB->cursor");
		return 2;
	}

	/* Initialize the key/data return pair. */
	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));

	/* Walk through the database and print out the key/data pairs. */
	while ((ret = cursor_directorios->c_get(cursor_directorios, &key, &data, DB_NEXT)) == 0) {

		list_add_directorio(directorio_deserealizar(data.data));
		cantidad_registros++;
	}
	cursor_directorios->close(cursor_directorios);

	return cantidad_registros;
}

int recuperar_archivos() {
	DBC *cursor_archivos;
	DBT key, data;
	int cantidad_registros = 0;
	int ret;

	/* Acquire a cursor for the database. */
	if ((ret = db_archivos->cursor(db_archivos, NULL, &cursor_archivos, 0)) != 0) {
		db_archivos->err(db_archivos, ret, "DB->cursor");
		return 2;
	}

	/* Initialize the key/data return pair. */
	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));

	/* Walk through the database and print out the key/data pairs. */
	while ((ret = cursor_archivos->c_get(cursor_archivos, &key, &data, DB_NEXT)) == 0) {
		t_archivo* archivo_deserealizado = archivo_deserealizar(data.data);
		archivo_deserealizado->disponible = false;
		list_add_archivo(archivo_deserealizado);
		cantidad_registros++;
	}
	cursor_archivos->close(cursor_archivos);

	return cantidad_registros;
}

int recuperar_nodos() {
	DBC *cursor_nodos;
	DBT key, data;
	int ret;

	/* Acquire a cursor for the database. */
	if ((ret = db_nodos->cursor(db_nodos, NULL, &cursor_nodos, 0)) != 0) {
		db_nodos->err(db_nodos, ret, "DB->cursor");
		return 2;
	}

	/* Initialize the key/data return pair. */
	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));

	/* Walk through the database and print out the key/data pairs. */
	while ((ret = cursor_nodos->c_get(cursor_nodos, &key, &data, DB_NEXT)) == 0) {
		t_nodo* nodo_deserealizado = nodo_deserealizar_db(data.data);
		list_add_nodos_aceptados(nodo_deserealizado);
	}
	cursor_nodos->close(cursor_nodos);

	return 0;
}

int insertar_directorio(t_directorio* directorio) {
	DBT key, data;
	int ret;
	db_recno_t recno = recno_directorios(false);

	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	key.data = &recno;
	key.size = sizeof(recno);

	directorio->id = recno;

	char* directorio_serializado = directorio_serializar(directorio);
	data.data = directorio_serializado;
	data.size = sizeof(t_directorio) - sizeof(pthread_rwlock_t);

	if ((ret = db_directorios->put(db_directorios, NULL, &key, &data, 0)) == 0) {
		log_debug_interno("BD: %d: key guardada", (int) key.data);
	} else {
		db_directorios->err(db_directorios, ret, "DB->put");
		return 2;
	}

	db_directorios->sync(db_directorios, 0);
	free(directorio_serializado);
	return 0;
}

int insertar_archivo(t_archivo* archivo) {
	DBT key, data;
	int ret;
	db_recno_t recno = recno_archivos(false);
	int bytes_serializados;

	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	key.data = &recno;
	key.size = sizeof(recno);

	archivo->id = recno;

	char* archivo_serializado = archivo_serializar(archivo, &bytes_serializados);
	data.data = archivo_serializado;
	data.size = bytes_serializados;

	if ((ret = db_archivos->put(db_archivos, NULL, &key, &data, 0)) == 0) {
		log_debug_interno("DB: %s: key guardada", (char *) key.data);
	} else {
		db_archivos->err(db_archivos, ret, "DB->put");
		return 2;
	}

	db_archivos->sync(db_archivos, 0);
	free(archivo_serializado);
	return 0;
}

int insertar_nodo(t_nodo* nodo) {
	DBT key, data;
	int ret;
	int bytes_serializados;

	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	key.data = nodo->nombre;
	key.size = sizeof(nodo->nombre);

	char* nodo_serializado = nodo_serializar_db(nodo, &bytes_serializados);
	data.data = nodo_serializado;
	data.size = bytes_serializados;

	if ((ret = db_nodos->put(db_nodos, NULL, &key, &data, 0)) == 0) {
		log_debug_interno("DB: %s: key guardada", (char *) key.data);
	} else {
		db_nodos->err(db_nodos, ret, "DB->put");
		return 2;
	}

	db_nodos->sync(db_nodos, 0);
	free(nodo_serializado);
	return 0;
}

int borrar_directorio(db_recno_t id) {
	DBT key;
	int ret;

	memset(&key, 0, sizeof(key));
	key.data = &id;
	key.size = sizeof(id);

	if ((ret = db_directorios->del(db_directorios, NULL, &key, 0)) == 0) {
		log_debug_interno("DB: %s: key borrada", (char *) key.data);
	} else {
		db_directorios->err(db_directorios, ret, "DB->del");
		return 2;
	}

	db_directorios->sync(db_directorios, 0);
	return 0;
}

int borrar_archivo(db_recno_t id) {
	DBT key;
	int ret;

	memset(&key, 0, sizeof(key));
	key.data = &id;
	key.size = sizeof(id);

	if ((ret = db_archivos->del(db_archivos, NULL, &key, 0)) == 0) {
		log_debug_interno("DB: %s: key borrada", (char *) key.data);
	} else {
		db_archivos->err(db_archivos, ret, "DB->del");
		return 2;
	}

	db_archivos->sync(db_archivos, 0);
	return 0;
}

int actualizar_directorio(t_directorio* directorio) {
	DBT key, data;
	int ret;
	db_recno_t recno = directorio->id;

	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	key.data = &recno;
	key.size = sizeof(recno);

	char* directorio_serializado = directorio_serializar(directorio);
	data.data = directorio_serializado;
	data.size = sizeof(t_directorio) - sizeof(pthread_rwlock_t);

	if ((ret = db_directorios->put(db_directorios, NULL, &key, &data, 0)) == 0) {
		log_debug_interno("DB: %d: key guardada", (int) key.data);
	} else {
		db_directorios->err(db_directorios, ret, "DB->put");
		return 2;
	}

	db_directorios->sync(db_directorios, 0);
	free(directorio_serializado);
	return 0;
}

int actualizar_archivo(t_archivo* archivo) {
	DBT key, data;
	int ret;
	db_recno_t recno = archivo->id;
	int bytes_serializados;

	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));
	key.data = &recno;
	key.size = sizeof(recno);

	char* archivo_serializado = archivo_serializar(archivo, &bytes_serializados);
	data.data = archivo_serializado;
	data.size = bytes_serializados;

	if ((ret = db_archivos->put(db_archivos, NULL, &key, &data, 0)) == 0) {
		log_debug_interno("DB: %s: key guardada", (char *) key.data);
	} else {
		db_archivos->err(db_archivos, ret, "DB->put");
		return 2;
	}

	db_archivos->sync(db_archivos, 0);
	free(archivo_serializado);
	return 0;
}

int cerrar_database() {
	db_archivos->close(db_archivos, 0);
	db_directorios->close(db_directorios, 0);
	db_nodos->close(db_nodos, 0);
	return 0;
}

db_recno_t recno_directorios(bool formatea) {
	static db_recno_t key_directorio = 0;
	pthread_mutex_lock(&mutex_recno_directorios);
	if (formatea) {
		key_directorio = 0;
	} else {
		key_directorio++;
	}
	pthread_mutex_unlock(&mutex_recno_directorios);
	return key_directorio;
}

db_recno_t recno_archivos(bool formatea) {
	static db_recno_t key_archivos = 0;
	pthread_mutex_lock(&mutex_recno_archivos);
	if (formatea) {
		key_archivos = 0;
	} else {
		key_archivos++;
	}
	pthread_mutex_unlock(&mutex_recno_archivos);
	return key_archivos;
}

