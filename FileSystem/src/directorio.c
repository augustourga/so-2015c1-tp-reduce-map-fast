#include "directorio.h"

t_directorio* directorio_crear() {
	t_directorio* directorio = malloc(sizeof(t_directorio));
	memset(directorio->nombre, 0, 255);
	directorio->padreId = 1;
	pthread_rwlock_init(&directorio->lock, NULL);
	return directorio;
}

t_directorio* directorio_crear_raiz() {
	t_directorio* raiz = directorio_crear();
	directorio_set_nombre(raiz, "/");
	directorio_set_padre(raiz, 0);
	raiz->id = 1;
	return raiz;
}

void directorio_set_nombre(t_directorio* directorio, char* nombre) {
	strcpy(directorio->nombre, nombre);
}

void directorio_set_padre(t_directorio* directorio, db_recno_t padreId) {
	directorio->padreId = padreId;
}

void directorio_eliminar(t_directorio* directorio) {
	free(directorio);
}

char* directorio_serializar(t_directorio* directorio) {

	char *directorio_serializado = malloc(sizeof(t_directorio) - sizeof(pthread_rwlock_t));

	int offset = 0;

	paquete_serializar(directorio_serializado, &directorio->id, sizeof(directorio->id), &offset);

	paquete_serializar(directorio_serializado, directorio->nombre, sizeof(directorio->nombre), &offset);

	paquete_serializar(directorio_serializado, &directorio->padreId, sizeof(directorio->padreId), &offset);

	return directorio_serializado;
}

t_directorio* directorio_deserealizar(char* directorio_serializado) {

	t_directorio* directorio = directorio_crear();

	int offset = 0;

	paquete_deserializar(&directorio->id, directorio_serializado, sizeof(directorio->id), &offset);

	paquete_deserializar(directorio->nombre, directorio_serializado, sizeof(directorio->nombre), &offset);

	paquete_deserializar(&directorio->padreId, directorio_serializado, sizeof(directorio->padreId), &offset);

	return directorio;
}

void log_error_ya_existe_directorio(char* directorio, char* directorio_padre_nuevo) {
	log_error_consola("Ya existe el directorio %s en %s", directorio, directorio_padre_nuevo);
}

void log_error_directorio_no_existe(char* ruta_directorio) {
	log_error_consola("El directorio %s no existe", ruta_directorio);
}

