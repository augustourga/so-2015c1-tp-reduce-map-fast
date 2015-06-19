#include "consola.h"

void iniciar_consola() {

	//En realidad no importa el tamaño del buffer porque si se pasa getLine se da cuenta y hace un realloc :D
	size_t buffer_size = 100;
	char* comando = (char *) calloc(1, buffer_size);
	puts("Ingrese un comando, ayuda o salir");
	while (!string_equals_ignore_case(comando, "salir\n")) {
		printf(">");
		int bytes_read = getline(&comando, &buffer_size, stdin);

		if (bytes_read == -1) {
			log_error_consola("Error en getline");
		}
		if (ejecutar_comando(comando) == 0) {
			char* comando_listo = comando_preparado(comando);
			log_debug_consola("El comando %s fue ejecutado con éxito", comando_listo);
		}
	}
	free(comando);
}

char* comando_preparado(char* comando) {
	char* comando_listo = calloc(1, strlen(comando));
	remueve_salto_de_linea(comando_listo, comando);
	return comando_listo;
}

int ejecutar_comando(char* comando) {
	int ret;
	char* comando_listo = comando_preparado(comando);
	char** parametros = string_n_split(comando_listo, 3, " ");
	if (string_equals_ignore_case(parametros[0], COMANDO_AYUDA)) {
		ret = mostrar_ayuda(parametros[1]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_FORMATEAR)) {
		ret = formatear_filesystem();
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_ELIMINAR_ARCHIVO)) {
		if (parametros[1] == NULL) {
			log_error_parametros_faltantes();
			return 1;
		}
		ret = eliminar_archivo(parametros[1]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_RENOMBRAR_ARCHIVO)) {
		if (parametros[1] == NULL || parametros[2] == NULL) {
			log_error_parametros_faltantes();
			return 1;
		}
		ret = renombrar_archivo(parametros[1], parametros[2]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_MOVER_ARCHIVO)) {
		if (parametros[1] == NULL || parametros[2] == NULL) {
			log_error_parametros_faltantes();
			return 1;
		}
		ret = mover_archivo(parametros[1], parametros[2]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_CREAR_DIRECTORIO)) {
		if (parametros[1] == NULL || parametros[2] == NULL) {
			log_error_parametros_faltantes();
			return 1;
		}
		ret = crear_directorio(parametros[1], parametros[2]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_ELIMINAR_DIRECTORIO)) {
		if (parametros[1] == NULL) {
			log_error_parametros_faltantes();
			return 1;
		}
		ret = eliminar_directorio(parametros[1]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_RENOMBRAR_DIRECTORIO)) {
		if (parametros[1] == NULL || parametros[2] == NULL) {
			log_error_parametros_faltantes();
			return 1;
		}
		ret = renombrar_directorio(parametros[1], parametros[2]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_MOVER_DIRECTORIO)) {
		if (parametros[1] == NULL || parametros[2] == NULL) {
			log_error_parametros_faltantes();
			return 1;
		}
		ret = mover_directorio(parametros[1], parametros[2]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_COPIAR_LOCAL_MDFS)) {
		if (parametros[1] == NULL || parametros[2] == NULL) {
			log_error_parametros_faltantes();
			return 1;
		}
		ret = copiar_archivo_local_a_mdfs(parametros[1], parametros[2]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_COPIAR_MDFS_LOCAL)) {
		if (parametros[1] == NULL || parametros[2] == NULL) {
			log_error_parametros_faltantes();
			return 1;
		}
		ret = copiar_archivo_mdfs_a_local(parametros[1], parametros[2]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_MD5)) {
		if (parametros[1] == NULL) {
			log_error_parametros_faltantes();
			return 1;
		}
		ret = md5(parametros[1]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_VER_BLOQUE)) {
		if (parametros[1] == NULL || parametros[2] == NULL) {
			log_error_parametros_faltantes();
			return 1;
		}
		ret = ver_bloque_de_archivo(strtol(parametros[1], NULL, 10), parametros[2]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_BORRAR_BLOQUE)) {
		if (parametros[1] == NULL || parametros[2] == NULL) {
			log_error_parametros_faltantes();
			return 1;
		}
		ret = borrar_bloque_de_nodo(strtol(parametros[1], NULL, 10), parametros[2]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_COPIAR_BLOQUE)) {
		parametros = string_n_split(comando_listo, 5, " ");
		if (parametros[1] == NULL || parametros[2] == NULL || parametros[3] == NULL) {
			log_error_parametros_faltantes();
			return 1;
		}
		ret = copiar_bloque_de_nodo_a_nodo(strtol(parametros[1], NULL, 10), parametros[2], parametros[3]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_AGREGAR_NODO)) {
		if (parametros[1] == NULL) {
			log_error_parametros_faltantes();
			return 1;
		}
		ret = agregar_nodo(parametros[1]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_ELIMINAR_NODO)) {
		if (parametros[1] == NULL) {
			log_error_parametros_faltantes();
			return 1;
		}
		ret = eliminar_nodo(parametros[1]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_LISTAR)) {
		if (parametros[1] == NULL) {
			log_error_parametros_faltantes();
			return 1;
		}
		ret = listar_hijos(parametros[1]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_SALIR)) {
		ret = cerrar_database();
		return 0;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_LISTAR_NODOS)) {
		if (parametros[1] == NULL) {
			log_error_parametros_faltantes();
			return 1;
		}
		ret = listar_nodos(parametros[1]);
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_DF)) {
		ret = espacio_libre();
		return ret;
	} else if (string_equals_ignore_case(parametros[0], COMANDO_DT)) {
		ret = espacio_total();
		return ret;
	} else {
		log_error_consola("Comando incorrecto ingrese ayuda para una lista de comandos válidos o salir para cerrar el sistema");
		return 1;
	}
	return 0;
}

void remueve_salto_de_linea(char* salida, char* texto) {
	strncpy(salida, texto, strlen(texto) - 1);
}

void imprimir_ruta_completa_de_directorio(t_directorio * directorio) {
	char ruta_completa[1024] = { 0 };
	t_directorio* directorio_actual = directorio;

	while (directorio_actual->padreId != 0) {
		char *tmp = strdup(ruta_completa);
		strcpy(ruta_completa, directorio_actual->nombre);
		strcat(ruta_completa, "/");
		strcat(ruta_completa, tmp);
		directorio_actual = directorio_por_id(directorio_actual->padreId);
		free(tmp);
	}
	printf("/%s\n", ruta_completa);
}

int listar_nodos(char* lista_nodos) {

	void _escribe_nombres_nodos(t_nodo* nodo) {
		printf("  %s - ", nodo->nombre);
		printf("BT: %d - ", nodo->cantidad_bloques_totales);
		printf("BL: %d - ", nodo->cantidad_bloques_libres);
		printf("BO: %d\n", nodo->cantidad_bloques_totales - nodo->cantidad_bloques_libres);
		if (nodo->puerto != 0) {
			printf("  Conexión: %s:%d\n", nodo->ip, nodo->puerto);
		}
	}

	if (string_equals_ignore_case(lista_nodos, "pendientes")) {
		list_iterate_nodos_pendientes((void *) _escribe_nombres_nodos);
	} else if (string_equals_ignore_case(lista_nodos, "operativos")) {
		list_iterate_nodos_operativos((void *) _escribe_nombres_nodos);
	} else if (string_equals_ignore_case(lista_nodos, "aceptados")) {
		list_iterate_nodos_aceptados((void *) _escribe_nombres_nodos);
	} else {
		log_error_consola("Parámetro incorrecto");
		return 1;
	}

	return 0;
}

int listar_hijos(char* ruta_directorio) {

	t_directorio* directorio_actual = NULL;

	if (string_equals_ignore_case(ruta_directorio, "/")) {
		directorio_actual = directorio_raiz();
	} else {
		directorio_actual = directorio_por_ruta(ruta_directorio);
	}

	if (directorio_actual == NULL) {
		log_error_directorio_no_existe(ruta_directorio);
		return 1;
	}

	t_list* directorios_hijos = directorios_hijos_de_directorio(directorio_actual);
	t_list* archivos_hijos = archivos_hijos_de_directorio(directorio_actual);
	imprimir_ruta_completa_de_directorio(directorio_actual);

	void escribe_nombres_directorios(t_directorio* directorio) {
		printf("  %s/\n", directorio->nombre);
	}

	list_iterate(directorios_hijos, (void*) escribe_nombres_directorios);

	void escribe_nombres_archivos(t_archivo* archivo) {
		char disponibilidad[16] = { 0 };
		if (!archivo->disponible) {
			strcpy(disponibilidad, "(no disponible)");
		}
		printf("  %s %s\n", archivo->nombre, disponibilidad);
	}

	list_iterate(archivos_hijos, (void*) escribe_nombres_archivos);

	list_destroy(archivos_hijos);
	list_destroy(directorios_hijos);

	return 0;
}

int mostrar_ayuda(char* parametro) {
	if (parametro == NULL) {
		//TODO: mostrar_ayuda
		puts("La ayuda");
	} else {
		printf("La ayuda de comando %s\n", parametro);
	}

	return 0;
}

int espacio_total() {
	char* mensaje_log = string_new();

	asprintf(&mensaje_log, "El espacio total del sistema es %dMB", cantidad_bloques_totales() * 20);
	puts(mensaje_log);
	log_info_interno(mensaje_log);
	free(mensaje_log);

	return 0;
}

int espacio_libre() {
	char* mensaje_log = string_new();

	asprintf(&mensaje_log, "El espacio libre del sistema es %dMB", cantidad_bloques_libres() * 20);
	puts(mensaje_log);
	log_info_interno("El espacio libre del sistema es %dMB", cantidad_bloques_libres() * 20);
	free(mensaje_log);

	return 0;
}

void log_error_parametros_faltantes() {
	log_error_consola("Falta ingresar parámetros");
}