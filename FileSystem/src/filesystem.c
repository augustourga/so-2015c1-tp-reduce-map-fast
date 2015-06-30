#include "filesystem.h"

#include <bits/mman-linux.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

t_list* lista_directorios;
t_list* lista_archivos;
t_list* lista_nodos_aceptados;
t_list* lista_nodos_operativos;
t_list* lista_nodos_pendientes;

pthread_mutex_t mutex_directorios = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_archivos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_nodos_operativos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_nodos_aceptados = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_nodos_pendientes = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_filesysem_operativo = PTHREAD_MUTEX_INITIALIZER;

bool filesystem_operativo = false;
int cantidad_nodos_minima = 0;

int crear_directorio(char* ruta_padre, char* nombre) {
	int ret;

	t_directorio* directorio_padre;

	if (string_equals_ignore_case(ruta_padre, "/")) {
		directorio_padre = directorio_raiz();
	} else {
		directorio_padre = directorio_por_ruta(ruta_padre);
	}

	if (directorio_padre == NULL) {
		log_error_directorio_no_existe(ruta_padre);
		return 1;
	}

	pthread_rwlock_rdlock(&directorio_padre->lock);

	if (existe_directorio_con_nombre_en_directorio(nombre, directorio_padre)) {
		log_error_ya_existe_directorio(nombre, directorio_padre->nombre);
		pthread_rwlock_unlock(&directorio_padre->lock);
		return 1;
	}

	t_directorio* directorio = directorio_crear();
	directorio_set_nombre(directorio, nombre);
	directorio_set_padre(directorio, directorio_padre->id);

	list_add_directorio(directorio);
	ret = insertar_directorio(directorio);

	pthread_rwlock_unlock(&directorio_padre->lock);
	return ret;
}

int renombrar_directorio(char* ruta_directorio, char* nombre_nuevo) {
	int ret;

	t_directorio* directorio;

	directorio = directorio_por_ruta(ruta_directorio);

	if (directorio == NULL) {
		log_error_directorio_no_existe(ruta_directorio);
		return 1;
	}
	pthread_rwlock_wrlock(&directorio->lock);

	t_directorio* directorio_padre = directorio_por_id(directorio->padreId);
	pthread_rwlock_rdlock(&directorio_padre->lock);

	if (existe_directorio_con_nombre_en_directorio(nombre_nuevo,
			directorio_padre)) {
		log_error_ya_existe_directorio(nombre_nuevo, directorio_padre->nombre);
		pthread_rwlock_unlock(&directorio->lock);
		pthread_rwlock_unlock(&directorio_padre->lock);
		return 1;
	}

	directorio_set_nombre(directorio, nombre_nuevo);
	ret = actualizar_directorio(directorio);

	pthread_rwlock_unlock(&directorio->lock);
	pthread_rwlock_unlock(&directorio_padre->lock);
	return ret;
}

int mover_directorio(char* ruta_directorio, char* ruta_padre_nuevo) {
	int ret;

	t_directorio* directorio;

	directorio = directorio_por_ruta(ruta_directorio);

	if (directorio == NULL) {
		log_error_directorio_no_existe(ruta_directorio);
		return 1;
	}
	pthread_rwlock_wrlock(&directorio->lock);

	t_directorio* directorio_padre_nuevo;

	directorio_padre_nuevo = directorio_por_ruta(ruta_padre_nuevo);

	if (directorio_padre_nuevo == NULL) {
		log_error_directorio_no_existe(ruta_padre_nuevo);
		return 1;
	}
	pthread_rwlock_rdlock(&directorio_padre_nuevo->lock);

	if (existe_directorio_con_nombre_en_directorio(directorio->nombre,
			directorio_padre_nuevo)) {
		log_error_ya_existe_directorio(ruta_directorio,
				directorio_padre_nuevo->nombre);
		pthread_rwlock_unlock(&directorio->lock);
		pthread_rwlock_unlock(&directorio_padre_nuevo->lock);
		return 1;
	}

	directorio_set_padre(directorio, directorio_padre_nuevo->id);
	ret = actualizar_directorio(directorio);

	pthread_rwlock_unlock(&directorio->lock);
	pthread_rwlock_unlock(&directorio_padre_nuevo->lock);
	return ret;
}

int eliminar_directorio(char* ruta_directorio) { //TODO: Ver de agregar un bool recursivo para hacer un rmr
	int ret;

	t_directorio* directorio;

	directorio = directorio_por_ruta(ruta_directorio);

	if (directorio == NULL) {
		log_error_directorio_no_existe(ruta_directorio);
		return 1;
	}
	pthread_rwlock_wrlock(&directorio->lock);

	if (directorio_tiene_directorios_hijos(directorio)
			|| directorio_tiene_archivos_hijos(directorio)) {
		log_error_consola("El directorio %s no está vacío", ruta_directorio);
		pthread_rwlock_unlock(&directorio->lock);
		return 1;
	}

	bool _directorio_por_id(t_directorio* directorio_actual) {
		return directorio_actual->id == directorio->id;
	}

	db_recno_t id_a_borrar = directorio->id;

	pthread_rwlock_unlock(&directorio->lock);
	list_remove_and_destroy_by_condition(lista_directorios,
			(void *) _directorio_por_id, (void *) directorio_eliminar);

	ret = borrar_directorio(id_a_borrar);
	return ret;
}

int copiar_archivo_local_a_mdfs(char* ruta_local, char* ruta_mdfs) {
	int num_block = 0;
	int offset_actual = 0;
	int pos_actual = 0;
	int ret;

	int fildes_local = open(ruta_local, O_RDONLY);

	if (fildes_local < 0) {
		log_error_archivo_no_existe(ruta_local);
		return 1;
	}

	t_directorio* directorio_padre;

	directorio_padre = directorio_por_ruta(ruta_mdfs);

	if (directorio_padre == NULL) {
		log_error_directorio_no_existe(ruta_mdfs);
		return 1;
	}
	pthread_rwlock_rdlock(&directorio_padre->lock);

	struct stat archivo_stat;
	stat(ruta_local, &archivo_stat);
	int archivo_size = archivo_stat.st_size;

	char** nombres = string_split(ruta_local, "/");
	char nombre_archivo[255];

	void _nombre_archivo(char* nombre) {
		strcpy(nombre_archivo, nombre);
	}

	string_iterate_lines(nombres, (void*) _nombre_archivo);

	if (existe_archivo_con_nombre_en_directorio(nombre_archivo,
			directorio_padre)) {
		log_error_consola("Ya existe el archivo %s", nombre_archivo);
		pthread_rwlock_unlock(&directorio_padre->lock);
		return 1;
	}

	t_archivo* archivo_nuevo = archivo_crear();
	archivo_set_nombre(archivo_nuevo, nombre_archivo);
	archivo_set_tamanio(archivo_nuevo, archivo_size);
	archivo_set_padre(archivo_nuevo, directorio_padre->id);

	char* map = mmap(0, archivo_size, PROT_READ, MAP_PRIVATE, fildes_local, 0);
	if (map == MAP_FAILED) {
		log_error_consola("Error de map");
		pthread_rwlock_unlock(&directorio_padre->lock);
		return 2;
	}

	t_chunk* chunks = malloc(sizeof(t_chunk));

	while (map[pos_actual] != 0) {
		int i;
		if (BLOQUE_SIZE_20MB + offset_actual <= archivo_size) {
			for (i = BLOQUE_SIZE_20MB + offset_actual; map[i] != 10; i--) {
				offset_actual = i;
			}
			chunks[num_block].inicio = map + pos_actual;
			chunks[num_block].tamanio = offset_actual - pos_actual;
			pos_actual = offset_actual;
		} else {
			//Significa que o entró en un solo bloque, o es el último bloque
			chunks[num_block].inicio = map + pos_actual;
			chunks[num_block].tamanio = archivo_size - offset_actual;
			pos_actual = archivo_size;
			break;
		}
		num_block++;
		chunks = realloc(chunks, (num_block + 1) * sizeof(t_chunk));
	}

	int cantidad_bloques = num_block + 1;
	archivo_nuevo->cantidad_bloques = cantidad_bloques;
	archivo_nuevo->bloques = malloc(cantidad_bloques * sizeof(t_bloque));

	if (cantidad_bloques * 3 > cantidad_bloques_libres()) {
		log_error_consola("No hay espacio suficiente para el archivo %s\n",
				archivo_nuevo->nombre);
		pthread_rwlock_unlock(&directorio_padre->lock);
		archivo_eliminar(archivo_nuevo);
		free(chunks);
		return 1;
	}

	int numero_bloque;
	t_nodo* nodo_actual;
	for (numero_bloque = 0; numero_bloque < cantidad_bloques; numero_bloque++) {
		t_nodo* nodos[3];
		if (proximos_nodos_disponibles(nodos)) {
			log_error_consola("No hay suficientes nodos disponibles");
			pthread_rwlock_unlock(&directorio_padre->lock);
			return 1;
		}
		int redundancia;
		archivo_nuevo->cantidad_copias_totales += 3;
		archivo_nuevo->bloques[numero_bloque].cantidad_copias = 3;
		archivo_nuevo->bloques[numero_bloque].copias =
				malloc(
						sizeof(t_copia)
								* archivo_nuevo->bloques[numero_bloque].cantidad_copias);
		for (redundancia = 0; redundancia < 3; redundancia++) {
			nodo_actual = nodos[redundancia];
			strcpy(
					archivo_nuevo->bloques[numero_bloque].copias[redundancia].nombre_nodo,
					nodo_actual->nombre);
			archivo_nuevo->bloques[numero_bloque].copias[redundancia].bloque_nodo =
					nodo_asignar_bloque_disponible(nodo_actual);
			archivo_nuevo->bloques[numero_bloque].copias[redundancia].conectado =
					true;
			archivo_nuevo->bloques[numero_bloque].copias[redundancia].tamanio_bloque =
					chunks[numero_bloque].tamanio;
			insertar_nodo(nodo_actual);
			pthread_rwlock_unlock(&(nodo_actual->lock));
			//enviar el chunk al bloque_disponible_de_nodo
		}
	}

	archivo_asignar_estado(archivo_nuevo, true);

	munmap(map, archivo_size);

	list_add_archivo(archivo_nuevo);

	ret = insertar_archivo(archivo_nuevo);
	pthread_rwlock_unlock(&directorio_padre->lock);

	close(fildes_local);
	free_puntero_puntero(nombres);
	return ret;
}

int copiar_archivo_mdfs_a_local(char* ruta_mdfs, char* ruta_local) {
	int fildes_local = open(ruta_local, O_RDWR | O_CREAT, S_IRWXU);

	if (fildes_local < 0) {
		log_error_consola("No se pudo crear el archivo %s", ruta_local);
		return 1;
	}

	t_archivo* archivo_mdfs;

	archivo_mdfs = archivo_por_ruta(ruta_mdfs);

	if (archivo_mdfs == NULL) {
		log_error_archivo_no_existe(ruta_mdfs);
		return 1;
	}

	pthread_rwlock_rdlock(&archivo_mdfs->lock);

	if (!archivo_mdfs->disponible) {
		log_error_consola("El archivo %s no está disponible", ruta_mdfs);
		pthread_rwlock_unlock(&archivo_mdfs->lock);
		return 1;
	}

	ftruncate(fildes_local, 160); //TODO: Ver de tomar el tamaño real del archivo (desde la base de datos que te dice el tamanio del archivo)
	int offset_actual = 0;

	char* map = mmap(0, 160, PROT_WRITE | PROT_READ, MAP_SHARED, fildes_local,
			0);
	if (map == MAP_FAILED) {
		log_error_consola("Error de map");
		pthread_rwlock_unlock(&archivo_mdfs->lock);
		return 2;
	}

	int numero_bloque;
	int numero_copia;

	struct arg_get_bloque args[archivo_mdfs->cantidad_bloques];
	pthread_t th_bloque[archivo_mdfs->cantidad_bloques];
	for (numero_bloque = 0; numero_bloque < archivo_mdfs->cantidad_bloques;
			numero_bloque++) {
		t_bloque bloque_actual = archivo_mdfs->bloques[numero_bloque];
		for (numero_copia = 0; numero_copia < bloque_actual.cantidad_copias;
				numero_copia++) {
			t_copia copia_actual = bloque_actual.copias[numero_copia];
			t_nodo* nodo;
			if (copia_actual.conectado) {
				nodo = nodo_operativo_por_nombre(copia_actual.nombre_nodo);
				if (nodo != NULL) {
					pthread_rwlock_rdlock(&nodo->lock);

					args[numero_bloque].bloque_nodo = copia_actual.bloque_nodo;
					args[numero_bloque].socket = nodo->socket;
					args[numero_bloque].nombre_nodo = strdup(
							copia_actual.nombre_nodo);

					pthread_create(&th_bloque[numero_bloque], NULL,
							(void *) mensaje_get_bloque,
							(void*) &args[numero_bloque]);

					pthread_rwlock_unlock(&nodo->lock);
				}
				break;
			}
		}
	}
	for (numero_bloque = 0; numero_bloque < archivo_mdfs->cantidad_bloques;
			numero_bloque++) {
		char* dato_bloque = malloc(10000);
		pthread_join(th_bloque[numero_bloque], (void*) &dato_bloque);

		if (dato_bloque == NULL) {
			log_error_consola("Ocurrió un error al generar el archivo");
			return 1;
		}

		int size = strlen(dato_bloque);
		memcpy(map + offset_actual, dato_bloque, size);
		//TODO: Ver si funciona
		//int size = strlen(copia_actual.nombre_nodo);
		//memcpy(map + offset_actual, copia_actual.nombre_nodo, size);
		offset_actual = offset_actual + size;
	}

	pthread_rwlock_unlock(&archivo_mdfs->lock);
	msync(map, 160, MS_SYNC);
	munmap(map, 160);

	return 0;
}

int md5(char* ruta_mdfs) {
	char** nombres = string_split(ruta_mdfs, "/");
	int i = 0;
	while (nombres[i + 1] != NULL) {
		i++;
	}

	char* ruta_temporal = malloc(260);
	strcpy(ruta_temporal, "/tmp/");
	strcat(ruta_temporal, nombres[i]);
	puts(ruta_temporal);
	copiar_archivo_mdfs_a_local(ruta_mdfs, ruta_temporal);

	char* comando = malloc(270);
	strcpy(comando, "md5sum ");
	strcat(comando, ruta_temporal);
	system(comando);

	free_puntero_puntero(nombres);
	return 0;
}

int renombrar_archivo(char* ruta_archivo, char* nombre_nuevo) {
	int ret;
	t_archivo* archivo;

	archivo = archivo_por_ruta(ruta_archivo);

	if (archivo == NULL) {
		log_error_archivo_no_existe(ruta_archivo);
		return 1;
	}
	pthread_rwlock_wrlock(&archivo->lock);

	t_directorio* directorio_padre = directorio_por_id(archivo->padreId);

	pthread_rwlock_rdlock(&directorio_padre->lock);

	if (existe_archivo_con_nombre_en_directorio(nombre_nuevo,
			directorio_padre)) {
		log_error_consola("Ya existe el archivo %s en el directorio indicado",
				nombre_nuevo);
		pthread_rwlock_unlock(&archivo->lock);
		pthread_rwlock_unlock(&directorio_padre->lock);
		return 1;
	}

	archivo_set_nombre(archivo, nombre_nuevo);

	ret = actualizar_archivo(archivo);

	pthread_rwlock_unlock(&archivo->lock);
	pthread_rwlock_unlock(&directorio_padre->lock);
	return ret;
}

int mover_archivo(char* ruta_archivo, char* ruta_padre_nuevo) {
	int ret;

	t_archivo* archivo;

	archivo = archivo_por_ruta(ruta_archivo);

	if (archivo == NULL) {
		log_error_archivo_no_existe(ruta_archivo);
		return 1;
	}

	pthread_rwlock_wrlock(&archivo->lock);

	t_directorio* directorio_padre_nuevo;

	directorio_padre_nuevo = directorio_por_ruta(ruta_padre_nuevo);

	if (directorio_padre_nuevo == NULL) {
		log_error_directorio_no_existe(ruta_padre_nuevo);
		pthread_rwlock_unlock(&archivo->lock);
		return 1;
	}

	pthread_rwlock_rdlock(&directorio_padre_nuevo->lock);

	if (existe_archivo_con_nombre_en_directorio(archivo->nombre,
			directorio_padre_nuevo)) {
		log_error_consola("Ya existe el archivo %s en el directorio indicado",
				archivo->nombre);
		pthread_rwlock_unlock(&archivo->lock);
		pthread_rwlock_unlock(&directorio_padre_nuevo->lock);
		return 1;
	}

	archivo_set_padre(archivo, directorio_padre_nuevo->id);

	ret = actualizar_archivo(archivo);

	pthread_rwlock_unlock(&archivo->lock);
	pthread_rwlock_unlock(&directorio_padre_nuevo->lock);
	return ret;
}

int eliminar_archivo(char* ruta_archivo) {
	int ret;

	t_archivo* archivo;

	archivo = archivo_por_ruta(ruta_archivo);

	if (archivo == NULL) {
		log_error_archivo_no_existe(ruta_archivo);
		return 1;
	}

	pthread_rwlock_wrlock(&archivo->lock);

	bool _archivo_por_id(t_archivo* archivo_actual) {
		return archivo_actual->id == archivo->id;
	}

	db_recno_t id_a_borrar = archivo->id;

	pthread_rwlock_unlock(&archivo->lock);

	list_remove_and_destroy_by_condition(lista_archivos,
			(void *) _archivo_por_id, (void *) archivo_eliminar);

	ret = borrar_archivo(id_a_borrar);
	return ret;
}

int inicializar_filesystem(bool formatea, int cantidad_nodos) {
	log_debug_consola("Inicializando Filesystem");
	lista_directorios = list_create();
	lista_archivos = list_create();
	lista_nodos_aceptados = list_create();
	lista_nodos_pendientes = list_create();
	lista_nodos_operativos = list_create();
	crear_directorio_raiz();
	crear_database();
	if (formatea) {
		recno_directorios(formatea);
		recno_archivos(formatea);
	}

	if (cantidad_nodos) {
		cantidad_nodos_minima = cantidad_nodos;
	}

	int cantidad_directorios = recuperar_directorios();
	int cantidad_archivos = recuperar_archivos();
	recuperar_nodos();

	log_debug_consola("Se recuperaron %d directorios y %d archivos de la DB",
			cantidad_directorios, cantidad_archivos);

	int i;
	for (i = 0; i < cantidad_directorios; i++) {
		recno_directorios(false);
	}

	for (i = 0; i < cantidad_archivos; i++) {
		recno_archivos(false);
	}
	return 0;
}

void crear_directorio_raiz() {
	t_directorio* raiz = directorio_crear_raiz();
	list_add_directorio(raiz);
	log_debug_consola("Se creó el directorio raiz");
}

t_directorio* directorio_raiz() {
	t_directorio* raiz = list_get(lista_directorios, 0);
	return raiz;
}

t_directorio* hijo_de_con_nombre(t_directorio* padre, char* nombre) {
	t_directorio* directorio_actual;

	bool _hijo_de_con_nombre(t_directorio* directorio) {
		return string_equals_ignore_case(directorio->nombre, nombre)
				&& directorio->padreId == padre->id;
	}

	directorio_actual = list_find_directorio((void*) _hijo_de_con_nombre);

	return directorio_actual;
}

t_archivo* archivo_hijo_de_con_nombre(t_directorio* padre, char* nombre) {
	t_archivo* archivo_actual;

	bool _archivo_hijo_de_con_nombre(t_archivo* archivo) {
		return string_equals_ignore_case(archivo->nombre, nombre)
				&& archivo->padreId == padre->id;
	}

	archivo_actual = list_find_archivo((void*) _archivo_hijo_de_con_nombre);

	return archivo_actual;
}

void free_puntero_puntero(char** lines) {
	string_iterate_lines(lines, (void*) free);
	free(lines);
}

t_directorio* directorio_por_ruta(char* ruta) {
	t_directorio* directorio_padre = directorio_raiz();

	char** nombres = string_split(ruta, "/");
	int i = 0;
	while (nombres[i] != NULL) {
		directorio_padre = hijo_de_con_nombre(directorio_padre, nombres[i]);
		i++;
	}
	free_puntero_puntero(nombres);
	return directorio_padre;
}

t_archivo* archivo_por_ruta(char* ruta) {
	t_archivo* archivo;
	t_directorio* directorio_padre = directorio_raiz();

	char** nombres = string_split(ruta, "/");
	int i = 0;
	while (nombres[i + 1] != NULL) {
		directorio_padre = hijo_de_con_nombre(directorio_padre, nombres[i]);
		i++;
	}

	archivo = archivo_hijo_de_con_nombre(directorio_padre, nombres[i]);
	free_puntero_puntero(nombres);

	return archivo;
}

t_directorio* directorio_por_id(db_recno_t id) {
	t_directorio* directorio;

	bool _directorio_por_id(t_directorio* directorio_actual) {
		return (directorio_actual->id == id);
	}

	directorio = list_find_directorio((void*) _directorio_por_id);

	return directorio;
}

bool existe_directorio_con_nombre_en_directorio(char* nombre,
		t_directorio* directorio_padre) {
	t_directorio* directorio;

	directorio = hijo_de_con_nombre(directorio_padre, nombre);

	return (directorio != NULL);
}

bool existe_archivo_con_nombre_en_directorio(char* nombre,
		t_directorio* directorio_padre) {
	t_archivo* archivo;

	archivo = archivo_hijo_de_con_nombre(directorio_padre, nombre);

	return (archivo != NULL);
}

bool directorio_tiene_directorios_hijos(t_directorio* padre) {
	t_directorio* directorio_actual;

	bool _hijo_de(t_directorio* directorio) {
		return directorio->padreId == padre->id;
	}

	directorio_actual = list_find_directorio((void*) _hijo_de);

	return directorio_actual != NULL;
}

bool directorio_tiene_archivos_hijos(t_directorio* padre) {
	t_archivo* archivo_actual;

	bool _hijo_de(t_archivo* archivo) {
		return archivo->padreId == padre->id;
	}

	archivo_actual = list_find_archivo((void*) _hijo_de);

	return archivo_actual != NULL;
}

t_list* directorios_hijos_de_directorio(t_directorio* directorio_actual) {

	bool _directorios_hijos_de_directorio(t_directorio* directorio) {
		return directorio->padreId == directorio_actual->id;
	}

	t_list* hijos = list_filter(lista_directorios,
			(void*) _directorios_hijos_de_directorio);

	return hijos;
}

t_list* archivos_hijos_de_directorio(t_directorio* directorio_actual) {

	bool _archivos_hijos_de_directorio(t_archivo* archivo) {
		return archivo->padreId == directorio_actual->id;
	}

	t_list* hijos = list_filter(lista_archivos,
			(void*) _archivos_hijos_de_directorio);

	return hijos;
}

int registrar_nodo(t_nodo* nodo) {
	int ret;
	t_nodo* nodo_existente;

	bool _nodo_por_nombre(t_nodo* nodo_actual) {
		return string_equals_ignore_case(nodo_actual->nombre, nodo->nombre);
	}

	nodo_existente = list_find_nodos_aceptados((void *) _nodo_por_nombre);

	if (nodo_existente == NULL) {
		// Es un nodo nuevo
		list_add_nodos_pendientes(nodo);
		log_info_nodo_conectado_nuevo(nodo);
		ret = 0;
	} else {
		ret = conexion_reconexion_nodo(nodo_existente, nodo);
	}
	return ret;
}

int conexion_reconexion_nodo(t_nodo* nodo_existente, t_nodo* nodo) {
	int ret;

	if (nodo != NULL) {
		//Como ya existía en la lista_nodos_agregados, se le pone el estado en true y se actualiza el socket
		pthread_rwlock_wrlock(&nodo_existente->lock);
		nodo_set_socket(nodo_existente, nodo->socket);
		strcpy(nodo_existente->ip, nodo->ip);
		nodo_existente->puerto = nodo->puerto;
		pthread_rwlock_unlock(&nodo_existente->lock);
	}
	list_add_nodos_operativos(nodo_existente);
	log_info_nodo_conectado_aceptado(nodo_existente);
	actualizar_disponibilidad_archivos_por_reconexion(nodo_existente);
	pasar_a_operativo();
	ret = insertar_nodo(nodo);

	return ret;
}

int agregar_nodo(char* nombre_nodo) {
	int ret;
	t_nodo* nodo = nodo_pendiente_por_nombre(nombre_nodo);

	if (nodo == NULL) {
		log_error_consola("El nodo %s no existe", nombre_nodo);
		return 1;
	}

	bool _nodo_por_nombre(t_nodo* nodo_actual) {
		return string_equals_ignore_case(nodo_actual->nombre, nombre_nodo);
	}

	pthread_mutex_lock(&mutex_nodos_pendientes);
	list_remove_by_condition(lista_nodos_pendientes, (void*) _nodo_por_nombre);
	pthread_mutex_unlock(&mutex_nodos_pendientes);

	list_add_nodos_aceptados(nodo);

	ret = conexion_reconexion_nodo(nodo, NULL);

	return ret;
}

int eliminar_nodo(char* nombre_nodo) {
	// Lo saca de los nodos operativos y lo pone en espera para que pueda ser agregado nuevamente
	t_nodo* nodo = nodo_aceptado_por_nombre(nombre_nodo);
	if (nodo == NULL) {
		log_error_consola("El nodo %s no existe", nombre_nodo);
		return 1;
	}

	bool _nodo_por_nombre(t_nodo* nodo_actual) {
		return string_equals_ignore_case(nodo_actual->nombre, nombre_nodo);
	}
	pthread_mutex_lock(&mutex_nodos_operativos);
	list_remove_by_condition(lista_nodos_operativos, (void *) _nodo_por_nombre);
	pthread_mutex_unlock(&mutex_nodos_operativos);
	actualizar_disponibilidad_archivos_por_desconexion(nodo);
	log_info_nodo_desconectado(nodo);
	list_add_nodos_pendientes(nodo);

	return 0;
}

t_nodo* nodo_aceptado_por_nombre(char* nombre_nodo) {
	t_nodo* nodo;

	bool _nodo_por_nombre(t_nodo* nodo_actual) {
		return string_equals_ignore_case(nodo_actual->nombre, nombre_nodo);
	}

	nodo = list_find_nodos_aceptados((void *) _nodo_por_nombre);

	return nodo;
}

t_nodo* nodo_pendiente_por_nombre(char* nombre_nodo) {
	t_nodo* nodo;

	bool _nodo_por_nombre(t_nodo* nodo_actual) {
		return string_equals_ignore_case(nodo_actual->nombre, nombre_nodo);
	}

	nodo = list_find_nodos_pendientes((void *) _nodo_por_nombre);

	return nodo;
}

t_nodo* nodo_pendiente_por_socket(int socket) {
	t_nodo* nodo;

	bool _nodo_por_socket(t_nodo* nodo_actual) {
		return nodo_actual->socket == socket;
	}

	nodo = list_find_nodos_pendientes((void *) _nodo_por_socket);

	return nodo;
}

t_nodo* nodo_operativo_por_nombre(char* nombre_nodo) {
	t_nodo* nodo;

	bool _nodo_por_nombre(t_nodo* nodo_actual) {
		return string_equals_ignore_case(nodo_actual->nombre, nombre_nodo);
	}

	nodo = list_find_nodos_operativos((void *) _nodo_por_nombre);

	return nodo;
}

t_nodo* nodo_operativo_por_socket(int socket) {
	t_nodo* nodo;

	bool _nodo_por_socket(t_nodo* nodo_actual) {
		return nodo_actual->socket == socket;
	}

	nodo = list_find_nodos_operativos((void *) _nodo_por_socket);

	return nodo;
}

int ver_bloque_de_archivo(int numero_bloque_archivo, char* ruta_archivo) {
	t_archivo* archivo;

	archivo = archivo_por_ruta(ruta_archivo);

	if (numero_bloque_archivo < 0) {
		log_error_consola("El número de bloque no puede ser negativo");
		return 1;
	}

	if (archivo == NULL) {
		log_error_archivo_no_existe(ruta_archivo);
		return 1;
	}
	pthread_rwlock_rdlock(&archivo->lock);

	if (numero_bloque_archivo > archivo->cantidad_bloques) {
		log_error_consola("El bloque %d no existe", numero_bloque_archivo);
		pthread_rwlock_unlock(&archivo->lock);
		return 1;
	}

	if (!archivo->disponible) {
		log_error_consola("El archivo %s no está disponible", ruta_archivo);
		return 1;
	}

	int redundancia = 0;
	while (!archivo->bloques[numero_bloque_archivo].copias[redundancia].conectado) {
		redundancia++;
	}
	t_copia copia_actual =
			archivo->bloques[numero_bloque_archivo].copias[redundancia];

	t_nodo* nodo = nodo_operativo_por_nombre(copia_actual.nombre_nodo);

	if (nodo == NULL) {
		log_error_consola("El nodo %s no existe", copia_actual.nombre_nodo);
		return 1;
	}

	pthread_rwlock_rdlock(&nodo->lock);
	pthread_t th_bloque;

	struct arg_get_bloque args;
	args.bloque_nodo = copia_actual.bloque_nodo;
	args.socket = nodo->socket;
	args.nombre_nodo = strdup(copia_actual.nombre_nodo);

	pthread_create(&th_bloque, NULL, (void *) mensaje_get_bloque,
			(void*) &args);
	char* dato_bloque = malloc(10000);
	pthread_join(th_bloque, (void*) &dato_bloque);

	if (dato_bloque == NULL) {
		log_error_consola("Ocurrió un error al generar el archivo");
		return 1;
	}

	printf("%s\n", dato_bloque);

	return 0;
}

int ver_bloque_de_nodo(int numero_bloque_nodo, char* nombre_nodo) {
	t_nodo* nodo;

	if (numero_bloque_nodo < 0) {
		log_error_consola("El número de bloque no puede ser negativo");
		return 1;
	}

	nodo = nodo_operativo_por_nombre(nombre_nodo);

	if (nodo == NULL) {
		log_error_consola("El nodo %s no está conectado", nombre_nodo);
		return 1;
	}
	pthread_rwlock_rdlock(&nodo->lock);

	if (numero_bloque_nodo > nodo->cantidad_bloques_totales) {
		log_error_consola("El nodo %s no está conectado", nombre_nodo);
		pthread_rwlock_unlock(&nodo->lock);
		return 1;
	}

	if (nodo->bloques[numero_bloque_nodo] == 0) {
		log_error_consola("El bloque %d está vacío", numero_bloque_nodo);
		pthread_rwlock_unlock(&nodo->lock);
		return 1;
	}

	//TODO: pedirle al nodo que me el contenido del bloque numero_bloque_nodo
	pthread_rwlock_unlock(&nodo->lock);
	return 0;
}

int borrar_bloque_de_nodo(int numero_bloque_nodo, char* nombre_nodo) {
	t_nodo* nodo;

	if (numero_bloque_nodo < 0) {
		log_error_consola("El número de bloque no puede ser negativo");
		return 1;
	}

	nodo = nodo_aceptado_por_nombre(nombre_nodo);

	if (nodo == NULL) {
		log_error_consola("El nodo %s no está aceptado", nombre_nodo);
		return 1;
	}
	pthread_rwlock_wrlock(&nodo->lock);

	if (numero_bloque_nodo > nodo->cantidad_bloques_totales) {
		log_error_consola("El bloque %d no existe", numero_bloque_nodo);
		pthread_rwlock_unlock(&nodo->lock);
		return 1;
	}

	if (nodo->bloques[numero_bloque_nodo] == 0) {
		log_error_consola("El bloque a borrar está vacío");
		pthread_rwlock_unlock(&nodo->lock);
		return 1;
	}
	nodo->bloques[numero_bloque_nodo] = 0;
	nodo->cantidad_bloques_libres++;
	insertar_nodo(nodo);
	pthread_rwlock_unlock(&nodo->lock);

	int numero_bloque_eliminado;
	int numero_copia_eliminada;

	bool _archivo_de_bloque(t_archivo* archivo) {
		int numero_bloque;
		int numero_copia;
		for (numero_bloque = 0; numero_bloque < archivo->cantidad_bloques;
				numero_bloque++) {
			t_bloque bloque_actual = archivo->bloques[numero_bloque];
			for (numero_copia = 0; numero_copia < bloque_actual.cantidad_copias;
					numero_copia++) {
				t_copia copia_actual = bloque_actual.copias[numero_copia];
				if (string_equals_ignore_case(copia_actual.nombre_nodo,
						nodo->nombre)
						&& copia_actual.bloque_nodo == numero_bloque_nodo) {
					numero_bloque_eliminado = numero_bloque;
					numero_copia_eliminada = numero_copia;
					return true;
				}
			}
		}
		return false;
	}

	t_archivo* archivo;
	archivo = list_find_archivo((void*) _archivo_de_bloque);

	if (archivo == NULL) {
		log_error_consola("Ocurrió un error");
		return 1;
	}

	pthread_rwlock_wrlock(&archivo->lock);

	t_bloque bloque_actual = archivo->bloques[numero_bloque_eliminado];
	t_copia* aux = calloc(
			archivo->bloques[numero_bloque_eliminado].cantidad_copias,
			sizeof(t_copia));

	memcpy(aux, bloque_actual.copias, sizeof(t_copia) * numero_copia_eliminada);

	memcpy(aux + numero_copia_eliminada,
			bloque_actual.copias + numero_copia_eliminada + 1,
			(bloque_actual.cantidad_copias - numero_copia_eliminada - 1)
					* sizeof(t_copia));

	archivo->cantidad_copias_totales--;
	archivo->bloques[numero_bloque_eliminado].cantidad_copias--;
	archivo->bloques[numero_bloque_eliminado].copias = aux;

	int numero_copia;
	bool deshabilitar = true;
	for (numero_copia = 0;
			numero_copia
					< archivo->bloques[numero_bloque_eliminado].cantidad_copias;
			numero_copia++) {
		t_copia copia_actual = bloque_actual.copias[numero_copia];
		if (copia_actual.conectado) {
			deshabilitar = false;
		}
	}

	if (deshabilitar) {
		archivo->disponible = false;
	}

	actualizar_archivo(archivo);

	pthread_rwlock_unlock(&archivo->lock);
	return 0;
}

int copiar_bloque_de_nodo_a_nodo(int numero_bloque_nodo,
		char* nombre_nodo_origen, char* nombre_nodo_destino) {
	t_nodo* nodo_origen;
	t_nodo* nodo_destino;

	if (numero_bloque_nodo < 0) {
		log_error_consola("El número de bloque no puede ser negativo");
		return 1;
	}

	nodo_origen = nodo_aceptado_por_nombre(nombre_nodo_origen);

	if (nodo_origen == NULL) {
		log_error_consola("El nodo %s no está aceptado", nombre_nodo_origen);
		return 1;
	}
	pthread_rwlock_rdlock(&nodo_origen->lock);

	if (numero_bloque_nodo > nodo_origen->cantidad_bloques_totales) {
		log_error_consola("El bloque %d no existe", numero_bloque_nodo);
		pthread_rwlock_unlock(&nodo_origen->lock);
		return 1;
	}

	if (nodo_origen->bloques[numero_bloque_nodo] == 0) {
		log_error_consola("El bloque a borrar está vacío");
		pthread_rwlock_unlock(&nodo_origen->lock);
		return 1;
	}

	nodo_destino = nodo_aceptado_por_nombre(nombre_nodo_destino);

	if (nodo_destino == NULL) {
		log_error_consola("El nodo %s no está aceptado", nombre_nodo_destino);
		pthread_rwlock_unlock(&nodo_origen->lock);
		return 1;
	}

	if (nodo_lleno(nodo_destino)) {
		log_error_consola("El nodo %s está lleno", nombre_nodo_destino);
		pthread_rwlock_unlock(&nodo_origen->lock);
		return 1;
	}

	pthread_rwlock_wrlock(&nodo_destino->lock);
	int numero_bloque_nodo_destino = nodo_asignar_bloque_disponible(
			nodo_destino);

	//enviar la info de un nodo a otro
	insertar_nodo(nodo_destino);

	pthread_rwlock_unlock(&nodo_destino->lock);
	pthread_rwlock_unlock(&nodo_origen->lock);

	int numero_bloque_copiado;
	int tamanio_bloque_copiado;

	bool _archivo_de_bloque(t_archivo* archivo) {
		int numero_bloque;
		int numero_copia;
		for (numero_bloque = 0; numero_bloque < archivo->cantidad_bloques;
				numero_bloque++) {
			t_bloque bloque_actual = archivo->bloques[numero_bloque];
			for (numero_copia = 0; numero_copia < bloque_actual.cantidad_copias;
					numero_copia++) {
				t_copia copia_actual = bloque_actual.copias[numero_copia];
				if (string_equals_ignore_case(copia_actual.nombre_nodo,
						nodo_origen->nombre)
						&& copia_actual.bloque_nodo == numero_bloque_nodo) {
					numero_bloque_copiado = numero_bloque;
					tamanio_bloque_copiado = copia_actual.tamanio_bloque;
					return true;
				}
			}
		}
		return false;
	}

	t_archivo* archivo;
	archivo = list_find_archivo((void*) _archivo_de_bloque);

	if (archivo == NULL) {
		log_error_consola("Ocurrió un error");
		return 1;
	}

	pthread_rwlock_wrlock(&archivo->lock);

	archivo->cantidad_copias_totales++;
	archivo->bloques[numero_bloque_copiado].cantidad_copias++;

	archivo->bloques[numero_bloque_copiado].copias = realloc(
			archivo->bloques[numero_bloque_copiado].copias,
			archivo->bloques[numero_bloque_copiado].cantidad_copias
					* sizeof(t_copia));

	archivo->bloques[numero_bloque_copiado].copias[archivo->bloques[numero_bloque_copiado].cantidad_copias
			- 1].conectado = true;
	archivo->bloques[numero_bloque_copiado].copias[archivo->bloques[numero_bloque_copiado].cantidad_copias
			- 1].bloque_nodo = numero_bloque_nodo_destino;
	archivo->bloques[numero_bloque_copiado].copias[archivo->bloques[numero_bloque_copiado].cantidad_copias
			- 1].tamanio_bloque = tamanio_bloque_copiado;
	strcpy(
			archivo->bloques[numero_bloque_copiado].copias[archivo->bloques[numero_bloque_copiado].cantidad_copias
					- 1].nombre_nodo, nombre_nodo_destino);

	actualizar_archivo(archivo);

	pthread_rwlock_unlock(&archivo->lock);
	return 0;
}

int cantidad_bloques_totales() {
	int bloques_totales = 0;

	void _suma_nodos_totales(t_nodo* nodo) {
		bloques_totales += nodo->cantidad_bloques_totales;
	}

	list_iterate_nodos_operativos((void *) _suma_nodos_totales);

	return bloques_totales;
}

int cantidad_bloques_libres() {
	int bloques_libres = 0;

	void _suma_nodos_totales(t_nodo* nodo) {
		bloques_libres += nodo->cantidad_bloques_libres;
	}

	list_iterate_nodos_operativos((void *) _suma_nodos_totales);

	return bloques_libres;
}

int proximos_nodos_disponibles(t_nodo* nodos[]) {

	pthread_mutex_lock(&mutex_nodos_operativos);
	bool _cantidad_bloques_libres(t_nodo* nodo1, t_nodo* nodo2) {
		return (nodo1->cantidad_bloques_totales - nodo1->cantidad_bloques_libres)
				< (nodo2->cantidad_bloques_totales
						- nodo2->cantidad_bloques_libres);
	}

	list_sort(lista_nodos_operativos, (void *) _cantidad_bloques_libres);
	t_list * lista_nodos_no_llenos = list_filter(lista_nodos_operativos,
			(void*) nodo_con_espacio);

	if (lista_nodos_no_llenos->elements_count < 3) {
		pthread_mutex_unlock(&mutex_nodos_operativos);
		return 1;
	}

	int redundancia;
	for (redundancia = 0; redundancia < 3; redundancia++) {
		nodos[redundancia] = list_get(lista_nodos_no_llenos, redundancia);
		pthread_rwlock_wrlock(&(nodos[redundancia]->lock));
	}
	pthread_mutex_unlock(&mutex_nodos_operativos);
	return 0;
}

void list_add_directorio(t_directorio* directorio) {
	pthread_mutex_lock(&mutex_directorios);
	list_add(lista_directorios, directorio);
	pthread_mutex_unlock(&mutex_directorios);
}

t_directorio* list_find_directorio(bool (*closure)(void*)) {
	t_directorio* directorio = NULL;

	pthread_mutex_lock(&mutex_directorios);
	directorio = list_find(lista_directorios, (void*) closure);
	pthread_mutex_unlock(&mutex_directorios);

	return directorio;
}

void list_add_archivo(t_archivo* archivo) {
	pthread_mutex_lock(&mutex_archivos);
	list_add(lista_archivos, archivo);
	pthread_mutex_unlock(&mutex_archivos);
}

t_archivo* list_find_archivo(bool (*closure)(void*)) {
	t_archivo* archivo = NULL;

	pthread_mutex_lock(&mutex_archivos);
	archivo = list_find(lista_archivos, (void*) closure);
	pthread_mutex_unlock(&mutex_archivos);

	return archivo;
}

//No me acuerdo para qué cree esta función pero por las dudas la dejo
t_archivo* list_find_archivo_disponible(bool (*closure)(void*)) {
	t_archivo* archivo;

	bool _archivo_disponible(t_archivo* archivo) {
		return archivo->disponible;
	}

	pthread_mutex_lock(&mutex_archivos);
	t_list* lista_archivos_disponibles = list_filter(lista_archivos,
			(void*) _archivo_disponible);
	pthread_mutex_unlock(&mutex_archivos);

	archivo = list_find(lista_archivos_disponibles, (void*) closure);

	return archivo;
}

void list_add_nodos_aceptados(t_nodo* nodo) {
	pthread_mutex_lock(&mutex_nodos_aceptados);
	list_add(lista_nodos_aceptados, nodo);
	pthread_mutex_unlock(&mutex_nodos_aceptados);
}

t_nodo* list_find_nodos_aceptados(bool (*closure)(void*)) {
	t_nodo* nodo = NULL;

	pthread_mutex_lock(&mutex_nodos_aceptados);
	nodo = list_find(lista_nodos_aceptados, (void*) closure);
	pthread_mutex_unlock(&mutex_nodos_aceptados);

	return nodo;
}

t_nodo* list_find_nodos_operativos(bool (*closure)(void*)) {
	t_nodo* nodo = NULL;

	pthread_mutex_lock(&mutex_nodos_operativos);
	nodo = list_find(lista_nodos_operativos, (void*) closure);
	pthread_mutex_unlock(&mutex_nodos_operativos);

	return nodo;
}

t_nodo* list_find_nodos_pendientes(bool (*closure)(void*)) {
	t_nodo* nodo = NULL;

	pthread_mutex_lock(&mutex_nodos_pendientes);
	nodo = list_find(lista_nodos_pendientes, (void*) closure);
	pthread_mutex_unlock(&mutex_nodos_pendientes);

	return nodo;
}

void list_add_nodos_pendientes(t_nodo* nodo) {
	pthread_mutex_lock(&mutex_nodos_pendientes);
	list_add(lista_nodos_pendientes, nodo);
	pthread_mutex_unlock(&mutex_nodos_pendientes);
}

void list_add_nodos_operativos(t_nodo* nodo) {
	pthread_mutex_lock(&mutex_nodos_operativos);
	list_add(lista_nodos_operativos, nodo);
	pthread_mutex_unlock(&mutex_nodos_operativos);
}

void list_iterate_nodos_pendientes(void (*closure)(void*)) {
	pthread_mutex_lock(&mutex_nodos_pendientes);
	list_iterate(lista_nodos_pendientes, (void *) closure);
	pthread_mutex_unlock(&mutex_nodos_pendientes);
}

void list_iterate_nodos_operativos(void (*closure)(void*)) {
	pthread_mutex_lock(&mutex_nodos_operativos);
	list_iterate(lista_nodos_operativos, (void *) closure);
	pthread_mutex_unlock(&mutex_nodos_operativos);
}

void list_iterate_nodos_aceptados(void (*closure)(void*)) {
	pthread_mutex_lock(&mutex_nodos_aceptados);
	list_iterate(lista_nodos_aceptados, (void *) closure);
	pthread_mutex_unlock(&mutex_nodos_aceptados);
}

void list_iterate_archivos(void (*closure)(void*)) {
	pthread_mutex_lock(&mutex_archivos);
	list_iterate(lista_archivos, (void *) closure);
	pthread_mutex_unlock(&mutex_archivos);
}

void desconectar_nodo(int socket) {
	t_nodo* nodo = nodo_operativo_por_socket(socket);
	bool _nodo_por_socket(t_nodo* nodo) {
		return nodo->socket == socket;
	}

	if (nodo != NULL) {
		pthread_mutex_lock(&mutex_nodos_operativos);
		list_remove_by_condition(lista_nodos_operativos,
				(void*) _nodo_por_socket);
		pthread_mutex_unlock(&mutex_nodos_operativos);
		actualizar_disponibilidad_archivos_por_desconexion(nodo);
		log_info_nodo_desconectado(nodo);
	} else {
		nodo = nodo_pendiente_por_socket(socket);
		if (nodo != NULL) {
			pthread_mutex_lock(&mutex_nodos_pendientes);
			list_remove_by_condition(lista_nodos_pendientes,
					(void*) _nodo_por_socket);
			pthread_mutex_unlock(&mutex_nodos_pendientes);
			log_info_nodo_desconectado(nodo);
		}
	}
}

void desconectar_marta(int socket) {
//TODO: desconectar_marta
}

void actualizar_disponibilidad_archivos_por_desconexion(t_nodo* nodo) {

	void _actualizar_disponibilidad(t_archivo* archivo) {
		int numero_bloque;
		int numero_copia;
		for (numero_bloque = 0; numero_bloque < archivo->cantidad_bloques;
				numero_bloque++) {
			t_bloque bloque_actual = archivo->bloques[numero_bloque];
			for (numero_copia = 0; numero_copia < bloque_actual.cantidad_copias;
					numero_copia++) {
				t_copia copia_actual = bloque_actual.copias[numero_copia];
				if (string_equals_ignore_case(copia_actual.nombre_nodo,
						nodo->nombre)) {
					archivo->bloques[numero_bloque].copias[numero_copia].conectado =
					false;
				}
			}
			bool deshabilitar = true;
			for (numero_copia = 0; numero_copia < bloque_actual.cantidad_copias;
					numero_copia++) {
				t_copia copia_actual = bloque_actual.copias[numero_copia];
				if (copia_actual.conectado) {
					deshabilitar = false;
					break;
				}
			}
			if (deshabilitar) {
				archivo->disponible = false;
				break;
			}
		}
	}

	list_iterate_archivos((void *) _actualizar_disponibilidad);
}

void actualizar_disponibilidad_archivos_por_reconexion(t_nodo* nodo) {

	void _actualizar_disponibilidad(t_archivo* archivo) {

		int numero_bloque;
		int numero_copia;
		bool habilitar = true;
		for (numero_bloque = 0; numero_bloque < archivo->cantidad_bloques;
				numero_bloque++) {
			t_bloque bloque_actual = archivo->bloques[numero_bloque];
			for (numero_copia = 0; numero_copia < bloque_actual.cantidad_copias;
					numero_copia++) {
				t_copia copia_actual = bloque_actual.copias[numero_copia];
				if (string_equals_ignore_case(copia_actual.nombre_nodo,
						nodo->nombre)) {
					archivo->bloques[numero_bloque].copias[numero_copia].conectado =
					true;
				}
			}
			bool salir = true;
			for (numero_copia = 0; numero_copia < bloque_actual.cantidad_copias;
					numero_copia++) {
				t_copia copia_actual = bloque_actual.copias[numero_copia];
				if (copia_actual.conectado) {
					salir = false;
					break;
				}
			}
			if (salir) {
				habilitar = false;
				break;
			}
		}
		if (habilitar) {
			archivo->disponible = true;
		}
	}

	list_iterate_archivos((void *) _actualizar_disponibilidad);
}

void pasar_a_operativo() {
	if (filesystem_operativo) {
		return;
	}

	pthread_mutex_lock(&mutex_filesysem_operativo);
	pthread_mutex_lock(&mutex_nodos_operativos);
	if (lista_nodos_operativos->elements_count >= cantidad_nodos_minima) {
		filesystem_operativo = true;
	}
	pthread_mutex_unlock(&mutex_nodos_operativos);
	pthread_mutex_unlock(&mutex_filesysem_operativo);
}

