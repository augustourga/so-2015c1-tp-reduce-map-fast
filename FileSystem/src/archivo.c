#include "archivo.h"

t_archivo* archivo_crear() {
	t_archivo* archivo = malloc(sizeof(t_archivo));
	memset(archivo->nombre, 0, 80);
	archivo->padreId = 1;
	archivo->tamanio = 0;
	archivo->cantidad_bloques = 0;
	archivo->disponible = false;
	pthread_rwlock_init(&archivo->lock, NULL);
	return archivo;
}

void archivo_set_nombre(t_archivo* archivo, char* nombre) {
	strcpy(archivo->nombre, nombre);
}

void archivo_set_padre(t_archivo* archivo, db_recno_t padreId) {
	archivo->padreId = padreId;
}

void archivo_set_tamanio(t_archivo* archivo, int tamanio) {
	archivo->tamanio = tamanio;
}

void archivo_asignar_estado(t_archivo* archivo, bool estado) {
	archivo->disponible = estado;
}

char* archivo_serializar(t_archivo* archivo, int* bytes_serializados) {

	(*bytes_serializados) = 0;

	char *archivo_serializado = malloc(
			sizeof(t_archivo) - sizeof(pthread_rwlock_t) + archivo->cantidad_bloques * sizeof(t_bloque) + archivo->cantidad_copias_totales * sizeof(t_copia));

	paquete_serializar(archivo_serializado, &(archivo->id), sizeof(archivo->id), bytes_serializados);

	paquete_serializar(archivo_serializado, archivo->nombre, sizeof(archivo->nombre), bytes_serializados);

	paquete_serializar(archivo_serializado, &(archivo->tamanio), sizeof(archivo->tamanio), bytes_serializados);

	paquete_serializar(archivo_serializado, &(archivo->padreId), sizeof(archivo->padreId), bytes_serializados);

	paquete_serializar(archivo_serializado, &(archivo->cantidad_bloques), sizeof(archivo->cantidad_bloques), bytes_serializados);

	paquete_serializar(archivo_serializado, &(archivo->cantidad_copias_totales), sizeof(archivo->cantidad_copias_totales), bytes_serializados);

	paquete_serializar(archivo_serializado, archivo->bloques, archivo->cantidad_bloques * sizeof(t_bloque), bytes_serializados);

	int numero_bloque;
	for (numero_bloque = 0; numero_bloque < archivo->cantidad_bloques; numero_bloque++) {
		t_bloque bloque_actual = archivo->bloques[numero_bloque];
		paquete_serializar(archivo_serializado, archivo->bloques[numero_bloque].copias, bloque_actual.cantidad_copias * sizeof(t_copia), bytes_serializados);
	}

	return archivo_serializado;
}

t_archivo* archivo_deserealizar(char* archivo_serializado) {

	t_archivo* archivo = archivo_crear();

	int offset = 0;
	int bytes_a_copiar;

	paquete_deserializar(&(archivo->id), archivo_serializado, sizeof(archivo->id), &offset);

	paquete_deserializar(archivo->nombre, archivo_serializado, sizeof(archivo->nombre), &offset);

	paquete_deserializar(&archivo->tamanio, archivo_serializado, sizeof(archivo->tamanio), &offset);

	paquete_deserializar(&archivo->padreId, archivo_serializado, sizeof(archivo->padreId), &offset);

	paquete_deserializar(&archivo->cantidad_bloques, archivo_serializado, sizeof(archivo->cantidad_bloques), &offset);

	paquete_deserializar(&archivo->cantidad_copias_totales, archivo_serializado, sizeof(archivo->cantidad_copias_totales), &offset);

	bytes_a_copiar = archivo->cantidad_bloques * sizeof(t_bloque);
	archivo->bloques = malloc(bytes_a_copiar);
	paquete_deserializar(archivo->bloques, archivo_serializado, bytes_a_copiar, &offset);

	int numero_bloque;
	int numero_copia;
	for (numero_bloque = 0; numero_bloque < archivo->cantidad_bloques; numero_bloque++) {
		t_bloque bloque_actual = archivo->bloques[numero_bloque];
		bytes_a_copiar = bloque_actual.cantidad_copias * sizeof(t_copia);
		archivo->bloques[numero_bloque].copias = malloc(bytes_a_copiar);
		paquete_deserializar(archivo->bloques[numero_bloque].copias, archivo_serializado, bytes_a_copiar, &offset);

		for (numero_copia = 0; numero_copia < bloque_actual.cantidad_copias; numero_copia++) {
			archivo->bloques[numero_bloque].copias[numero_copia].conectado = false;
		}
	}

	return archivo;
}

void log_error_archivo_no_existe(char* ruta_local) {
	log_error_consola("El archivo %s no existe", ruta_local);
}
