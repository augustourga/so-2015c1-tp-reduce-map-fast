#include "Nodo.h"

int main(int argc, char*argv[]) {

	log_crear("INFO", LOG_FILE, PROCESO);

	if (levantarConfiguracionNodo()) {
		log_error_consola("Hubo errores en la carga de las configuraciones.");
		exit(1);
	}

	_data = levantar_espacio_datos();
	archivos_temporales = list_create();

	if (levantarHiloFile()) {
		log_error_consola("Conexion con File System fallida.");
		exit(1);
	}

	levantarNuevoServer();

	liberar_Espacio_datos(_data, ARCHIVO_BIN);

	return 0;
}

int levantarConfiguracionNodo() {
//	char* aux;
	t_config* archivo_config = config_create(PATH_CONFIG);

	PUERTO_FS = config_get_int_value(archivo_config, "PUERTO_FS");
	IP_FS = strdup(config_get_string_value(archivo_config, "IP_FS"));
	ARCHIVO_BIN = strdup(config_get_string_value(archivo_config, "ARCHIVO_BIN"));
	DIR_TEMP = strdup(config_get_string_value(archivo_config, "DIR_TEMP"));
	PUERTO_NODO = config_get_int_value(archivo_config, "PUERTO_NODO");
	NOMBRE_NODO = strdup(config_get_string_value(archivo_config, "NOMBRE_NODO"));
	config_destroy(archivo_config);
//
//	if (strncmp(aux, "SI", 2) == 0) {
//		NODO_NUEVO = 1;
//	} else {
//		NODO_NUEVO = 0;
//	}

	return 0;
}

int levantarHiloFile() {
	pthread_t thr_Conexion_FileSystem;
	t_conexion_nodo* reg_conexion = malloc(sizeof(t_conexion_nodo));
	rcx = pthread_create(&thr_Conexion_FileSystem, NULL, (void *) conectarFileSystem, reg_conexion);
	if (rcx != 0) {
		log_error_consola("El thread que acepta las conexiones entrantes no pudo ser creado.");
	}
	return 0;
}

void conectarFileSystem(t_conexion_nodo* reg_conexion) {

	reg_conexion->sock_fs = client_socket(IP_FS, PUERTO_FS);
	t_msg* mensaje = string_message(CONEXION_NODO, NOMBRE_NODO, 1, CANT_BLOQUES);
	enviar_mensaje(reg_conexion->sock_fs, mensaje);
	destroy_message(mensaje);
	log_info_interno("Conectado al File System en el socket %d", reg_conexion->sock_fs);

	while (true) {
		char*bloque = NULL;
		t_msg* mensaje2;
 		t_msg*codigo;
		if((codigo= recibir_mensaje(reg_conexion->sock_fs))!=NULL){

		switch (codigo->header.id) {

		case GET_BLOQUE:

			bloque = getBloque(codigo->argv[0]);
			mensaje2 = string_message(GET_BLOQUE_OK, bloque, 1, codigo->argv[1]);
			enviar_mensaje(reg_conexion->sock_fs, mensaje2);
			free(bloque);
			destroy_message(mensaje2);
			destroy_message(codigo);
			break;

		case SET_BLOQUE:

			bloque = malloc(tamanio_bloque);
			memcpy(bloque, codigo->stream, codigo->argv[1]); //1 es el tamaño real, el stream es el bloque de 20mb(aprox)
			memset(bloque + codigo->argv[1], '\0', tamanio_bloque - codigo->argv[1]);
			setBloque(codigo->argv[0], bloque);
			mensaje2 = id_message(SET_BLOQUE_OK);
			enviar_mensaje(reg_conexion->sock_fs, mensaje2);
			free(bloque);
			destroy_message(codigo);
			break;

		case GET_FILE_CONTENT:

			bloque = getFileContent(codigo->stream);
			mensaje2 = id_message(GET_FILE_CONTENT);
			mensaje2->stream = bloque;
			enviar_mensaje(reg_conexion->sock_fs, mensaje2);
			destroy_message(mensaje2);
			destroy_message(codigo);
			free(bloque);
			break;
		default:
			log_error_interno("Mensaje Incorrecto. Se esperaba GET_FILE_CONTEN | SET_BLOQUE | GET_BLOQUE");
			break;
		}
		}
		else{ log_error_interno("Se ha desconectado el File System");
		exit(1);
		}

	}
}

void levantarNuevoServer() {
	pthread_t thread;
	int listener, nuevaConexion;
	listener = server_socket(PUERTO_NODO);
	log_info_interno("Escuchando conexiones entrantes");

	while (true) {
		nuevaConexion = accept_connection(listener);
		if(nuevaConexion>= 0){
			log_info_interno("Se ha conectado un proceso al socket %d", nuevaConexion);
			pthread_create(&thread, NULL, atenderConexiones, &nuevaConexion);
		}
		}
}

void *atenderConexiones(void *parametro) {
	t_msg *codigo;
	int sock_conexion = *((int *) parametro);
	char*bloque = NULL;
	t_msg* mensaje2;
	t_queue *cola_nodos = queue_create();
	t_msg_id fin;
	t_nodo_archivo* nodo_arch;
	char*rutina_reduce;

	while (1) {
		if ((codigo = recibir_mensaje(sock_conexion)) != NULL) {

			switch (codigo->header.id) {

			case GET_BLOQUE:
				bloque = getBloque(codigo->argv[0]);
				mensaje2 = string_message(GET_BLOQUE_OK, bloque, 1,
						codigo->argv[1]);
				enviar_mensaje(sock_conexion, mensaje2);
				free(bloque);
				destroy_message(mensaje2);
				destroy_message(codigo);
				break;

			case SET_BLOQUE:
				bloque = malloc(tamanio_bloque);
				memcpy(bloque, codigo->stream, codigo->argv[1]); //1 es el tamaño real, el stream es el bloque de 20mb(aprox)
				memset(bloque + codigo->argv[1], '\0',
						tamanio_bloque - codigo->argv[1]);
				setBloque(codigo->argv[0], bloque);
				mensaje2 = id_message(SET_BLOQUE_OK);
				enviar_mensaje(sock_conexion, mensaje2);
				free(bloque);
				destroy_message(codigo);
				break;

			case GET_FILE_CONTENT:

				bloque = getFileContent(codigo->stream);

				mensaje2 = id_message(GET_FILE_CONTENT);
				mensaje2->stream = bloque;

				enviar_mensaje(sock_conexion, mensaje2);
				destroy_message(mensaje2);
				destroy_message(codigo);
				free(bloque);
				break;

			case EJECUTAR_MAP:

				mensaje2 = recibir_mensaje(sock_conexion);
				if(mensaje2->header.id == RUTINA){
				fin = ejecutar_map(mensaje2->stream, codigo->stream, codigo->argv[0], mensaje2->argv[0]);
				switch (fin) {
				case FIN_MAP_ERROR:
					log_error_interno("Hubo errores en el map %d en el bloque %d.", mensaje2->argv[0], codigo->argv[0]);
					break;
				case FIN_MAP_OK:
					log_info_interno("Map %d en el bloque %d exitoso", mensaje2->argv[0], codigo->argv[0]);
					break;
				default:
					log_info_interno("Mensaje incorrecto se esperaba el id FIN_MAP_OK ó FIN_MAP_ERROR");
					break;

				}
				mensaje2 = id_message(fin);
				enviar_mensaje(sock_conexion, mensaje2);
				}else{log_error_interno("Fallo en Recibir Rutina. Se esperaba el id RUTINA.");}

				destroy_message(mensaje2);
				destroy_message(codigo);


				break;

			case EJECUTAR_REDUCE:

				//Se recibe la rutina
				mensaje2 = recibir_mensaje(sock_conexion);

				if(mensaje2->header.id == RUTINA){
					size_t tamanioEjecutable = strlen(mensaje2->stream);

				rutina_reduce = malloc(tamanioEjecutable);
				memcpy(rutina_reduce, mensaje2->stream, tamanioEjecutable);

				while (mensaje2->header.id != FIN_ENVIO_MENSAJE) {
					mensaje2 = recibir_mensaje(sock_conexion);
					nodo_arch = (t_nodo_archivo*) malloc(sizeof(t_nodo_archivo));
					nodo_arch->ip = string_n_split(mensaje2->stream, 2, "|")[0];
					nodo_arch->archivo = string_n_split(mensaje2->stream, 2, "|")[1];
					nodo_arch->puerto = mensaje2->argv[0];
					queue_push(cola_nodos, (void*) nodo_arch);

				}

				fin = ejecutar_reduce(rutina_reduce, codigo->stream, cola_nodos, codigo->argv[0]);
				mensaje2 = id_message(fin);
				enviar_mensaje(sock_conexion, mensaje2);

				free(rutina_reduce);
				}
				else{ log_error_interno("Fallo en Recibir Rutina.Se esperaba el id RUTINA.");}
				destroy_message(mensaje2);
				destroy_message(codigo);
				break;

			case GET_NEXT_ROW:
				bloque = obtener_proximo_registro_de_archivo(codigo->stream);
				if (bloque != NULL) {
					mensaje2 = string_message(GET_NEXT_ROW_OK, bloque, 0);
					log_info_interno("GET_NEXT_ROW_OK");

				} else {
					mensaje2 = id_message(GET_NEXT_ROW_ERROR);
					log_info_interno("GET_NEXT_ROW_ERROR");
				}
				enviar_mensaje(sock_conexion, mensaje2);
				destroy_message(mensaje2);
				destroy_message(codigo);

				break;
			default:
				log_info_interno("Mensaje incorrecto.Se esperaba GET_BLOQUE|SET_BLOQUE|GET_FILE_CONTENT| EJECUTAR_MAP| EJECUTAR_REDUCE | GET_NEXT_ROW");
				break;
			}

		} else {
			log_error_interno("Se ha desconectado un proceso del socket %d", sock_conexion);


		}
		break;}
	return NULL;
}

void setBloque(int numeroBloque, char* bloque_datos) {
	//debo pararme en la posicion donde se encuentra almacenado el bloque y empezar a grabar
	log_info_interno("Inicio setBloque(%d)", numeroBloque);
	memcpy(_data + (numeroBloque * tamanio_bloque), bloque_datos, tamanio_bloque);
	log_info_interno("Fin setBloque(%d)", numeroBloque);

}

char* getBloque(int numeroBloque) {
	log_info_interno("Ini getBloque(%d)", numeroBloque);
	char* bloque = NULL;
	bloque = malloc(tamanio_bloque);
	memcpy(bloque, &(_data[numeroBloque * tamanio_bloque]), tamanio_bloque);
	log_info_interno("Fin getBloque(%d)", numeroBloque);
	return bloque;
}

char* getFileContent(char* filename) {

	log_info_interno("Inicio getFileContent(%s)", filename);
	char* content = NULL;
	//creo el espacio para almacenar el archivo
	char* path = file_combine(DIR_TEMP, filename);
	size_t size = file_get_size(path) + 1;
	content = malloc(size);
	char* mapped = NULL;
	mapped = file_get_mapped(path);
	memcpy(content, mapped, size);        //
	file_mmap_free(mapped, path);
	free_null((void*) &path);
	log_info_interno("Fin getFileContent(%s)", filename);
	return content;

}

char* levantar_espacio_datos() {
	CANT_BLOQUES = file_get_size(ARCHIVO_BIN) / tamanio_bloque;
	log_info_interno("Levantado %s. Cantidad de bloque:%d ",ARCHIVO_BIN,CANT_BLOQUES);
	return file_get_mapped(ARCHIVO_BIN);
}


void liberar_Espacio_datos(char* _data, char* ARCHIVO) {
	char* path = file_combine(DIR_TEMP, ARCHIVO);
	file_mmap_free(_data, path);
}

t_msg_id ejecutar_map(char*ejecutable, char* nombreArchivoFinal, int numeroBloque, int mapid) {
	log_info_interno("Inicio ejecutarMap ID:%d en el bloque %d", mapid, numeroBloque);
	char*bloque = NULL;
	bloque = getBloque(numeroBloque);
	char* temporal = generar_nombre_temporal(mapid, "map", numeroBloque);
	char*ruta_sort = "/usr/bin/sort";
	char* path_ejecutable = generar_nombre_rutina("map");
	write_file(path_ejecutable, ejecutable, strlen(ejecutable));
	chmod(path_ejecutable, S_IRWXU);
	if (ejecutar(bloque, path_ejecutable, temporal)) {
		return FIN_MAP_ERROR;
	}
	char* data = read_whole_file(temporal);
	if (ejecutar(data, ruta_sort, nombreArchivoFinal)) {
		return FIN_MAP_ERROR;
	}
	list_add_archivo_tmp(nombreArchivoFinal);
	remove(path_ejecutable);
	remove(temporal);
	free(bloque);
	log_info_interno("Fin ejecutarMap ID:%d en el bloque %d", mapid, numeroBloque);
	return FIN_MAP_OK;

}

t_msg_id ejecutar_reduce(char*ejecutable, char* nombreArchivoFinal, t_queue* colaArchivos, int id_reduce) {

	log_info_interno("Inicio ejecutar Reduce ID:%d ", id_reduce);

	t_list* lista_nodos;

	char* path_ejecutable = generar_nombre_rutina("reduce");
	write_file(path_ejecutable, ejecutable, strlen(ejecutable));
	chmod(path_ejecutable, S_IRWXU);
	char*temporal = generar_nombre_temporal(id_reduce, "reduce", 667);

	lista_nodos = deserealizar_cola(colaArchivos);
	int res;
	res = apareo(temporal, lista_nodos);
	if (res == -1) {
		remove(temporal);
		return FIN_REDUCE_ERROR;
	}
	char* data = read_whole_file(temporal);
	if (ejecutar(data, path_ejecutable, nombreArchivoFinal)) {
		remove(temporal);
		return FIN_REDUCE_ERROR;
	}
	remove(temporal);
	log_info_interno("Fin ejecutar Reduce ID:%d ", id_reduce);
	return FIN_REDUCE_OK;
}

t_list* deserealizar_cola(t_queue* colaArchivos) {
	int k, a;
	char** lista_archivos_aux;
	t_nodo_archivo* nodo_aux = (t_nodo_archivo*) malloc(sizeof(t_nodo_archivo));
	t_list* lista_nodos = list_create();
	t_nodo_archivo* elem = (t_nodo_archivo*) malloc(sizeof(t_nodo_archivo));
	elem = (t_nodo_archivo *) queue_pop(colaArchivos);
	lista_archivos_aux = string_split(elem->archivo, ";");
	for (k = 0; lista_archivos_aux[k] != NULL; k++) {
		nodo_aux->ip = NULL;
		nodo_aux->puerto = elem->puerto;
		nodo_aux->archivo = lista_archivos_aux[k];
		list_add(lista_nodos, nodo_aux);
	}
	a = queue_size(colaArchivos);
	if (a != 0) {
		elem = (t_nodo_archivo *) queue_pop(colaArchivos);
		while (a != 0) {
			lista_archivos_aux = string_split(elem->archivo, ";");
			for (k = 0; lista_archivos_aux[k] != NULL; k++) {
				nodo_aux->ip = elem->ip;
				nodo_aux->puerto = elem->puerto;
				nodo_aux->archivo = lista_archivos_aux[k];
				list_add(lista_nodos, nodo_aux);
			}
			a = queue_size(colaArchivos);
			elem = (t_nodo_archivo *) queue_pop(colaArchivos);

		}
	}
	free(elem);
	return lista_nodos;
}

int apareo(char* temporal, t_list* lista_nodos_archivos) {
	char** registros = malloc(sizeof(int));
	int i, pos, valor_actual_1, valor_actual_2;
	int res = 0;
	char* clave_actual_1 = string_new();
	char* clave_actual_2 = string_new();
	char* registro_actual_1 = string_new();
	char* registro_actual_2 = string_new();
	char* aux_string;

	char* ruta = file_combine(DIR_TEMP, temporal);
	FILE* archivo = fopen(ruta, "ab");

	// Obtengo el primer registro de cada archivo
	int cantidad_nodos_archivos = list_size(lista_nodos_archivos);
	t_nodo_archivo* elem = (t_nodo_archivo*) malloc(sizeof(t_nodo_archivo));
	for (i = 0; i < cantidad_nodos_archivos; i++) {
		elem = (t_nodo_archivo *) list_get(lista_nodos_archivos, i);
		aux_string = obtener_proximo_registro(elem);
		if (string_equals_ignore_case(aux_string, "error.rmf")) {
			res = -1;
			return res;
		} else {
			registros[i] = aux_string;
		}
	}

	pos = obtener_posicion_menor_clave(registros, cantidad_nodos_archivos);
	strcpy(registro_actual_1, registros[pos]);
	elem = (t_nodo_archivo *) list_get(lista_nodos_archivos, pos);
	aux_string = obtener_proximo_registro(elem);
	if (string_equals_ignore_case(aux_string, "error.rmf")) {
		res = -1;
		return res;
	} else {
		registros[pos] = aux_string;
	}
	char** array_aux = string_n_split(registro_actual_1, 2, ";");
	strcpy(clave_actual_1, array_aux[0]);
	valor_actual_1 = atoi(array_aux[1]);

	pos = obtener_posicion_menor_clave(registros, cantidad_nodos_archivos); // esta pos se obtiene para ya tener un 2do valor antes de entrar al while
	// obtener_posicion_menor_clave devuelve -1 una vez que en el array ya son todos campos nulos u EOF
	while (pos != -1) {
		strcpy(registro_actual_2, registros[pos]);
		elem = (t_nodo_archivo *) list_get(lista_nodos_archivos, pos);
		aux_string = obtener_proximo_registro(elem);
		if (string_equals_ignore_case(aux_string, "error.rmf")) {
			res = -1;
			return res;
		} else {
			registros[pos] = aux_string;
		}
		char** array_aux = string_n_split(registro_actual_2, 2, ";");
		strcpy(clave_actual_2, array_aux[0]);
		valor_actual_2 = atoi(array_aux[1]);

		if (strcmp(clave_actual_1, clave_actual_2) == 0) {
			valor_actual_1 = valor_actual_1 + valor_actual_2;
		} else {
			string_append(&clave_actual_1, ";");
			string_append(&clave_actual_1, string_itoa(valor_actual_1));
			string_append(&clave_actual_1, "\n");
			if (archivo != NULL) {
				fputs(clave_actual_1, archivo);
			} else {
				log_error_consola("No se pudo acceder al archivo temporal para guardar data de apareamiento");
				res = -1;
				return res;
			}
			strcpy(clave_actual_1, clave_actual_2);
			valor_actual_1 = valor_actual_2;
		}
		pos = obtener_posicion_menor_clave(registros, cantidad_nodos_archivos);
	}

	// guardo en el archivo el ultimo registro con el que estaba trabajando para no perderlo
	string_append(&clave_actual_1, ";");
	string_append(&clave_actual_1, string_itoa(valor_actual_1));
	string_append(&clave_actual_1, "\n");
	if (archivo != NULL) {
		fputs(clave_actual_1, archivo);
	} else {
		log_error_consola("No se pudo acceder al archivo temporal para guardar data de apareamiento");
		res = -1;
		return res;
	}
	// TODO: hay q ver de liberar todas las variables usadas aca
	fclose(archivo);
	log_info_interno("Se aparearon correctamente los archivos.");
	return res;
}

char* obtener_proximo_registro(t_nodo_archivo* nodo_archivo) {
	char* resultado;
	if (nodo_archivo->ip == NULL) {
		resultado = obtener_proximo_registro_de_archivo(nodo_archivo->archivo);
	} else {
		resultado = enviar_mensaje_proximo_registro(nodo_archivo);
	}
	return resultado;
}

int obtener_posicion_menor_clave(char** registros, int cantidad_nodos_archivos) {
	int pos, i, aux;
	char* clave_1;
	char* clave_2;

	// obtiene primer campo != NULL
	for (pos = 0; (registros[pos] != NULL) && (string_is_empty(clave_1) && (pos < cantidad_nodos_archivos)); pos++) {
		clave_1 = string_n_split(registros[pos], 2, ";")[0];
	}

	if (!string_is_empty(clave_1)) {
		for (i = pos + 1; i < cantidad_nodos_archivos; i++) {
			clave_2 = string_n_split(registros[i], 2, ";")[0];
			aux = strcmp(clave_1, clave_2);
			if (aux > 0) {
				pos = i;
				strcpy(clave_1, clave_2);
			}
		}
	} else {
		// devuelve -1 una vez que en el array ya son todos campos nulos
		pos = -1;
	}
	return pos;
}

char* obtener_proximo_registro_de_archivo(char* archivo) {
	fpos_t* posicion_puntero = obtener_posicion_puntero_arch_tmp(archivo);
	int bytes_read;
	size_t buffer_size = 100;
	char* linea = (char *) calloc(1, buffer_size);

	FILE* file = fopen(archivo, "r");
	if (file != NULL) {
		if (posicion_puntero != NULL) {
			fsetpos(file, posicion_puntero);
		}
		bytes_read = getline(&linea, &buffer_size, file);
		if (bytes_read == -1) {
			linea = NULL;
		} else {
			fgetpos(file, posicion_puntero);
			actualizar_posicion_puntero_arch_tmp(archivo, posicion_puntero);
		}
		fclose(file);
	} else {
		log_error_consola("No pudo abrirse el archivo temporal");
		linea = NULL;
	}
	return linea;
}

char* enviar_mensaje_proximo_registro(t_nodo_archivo* nodo_archivo) {
	char* resultado = string_new();
	int socket_tmp;
	socket_tmp = client_socket(nodo_archivo->ip, nodo_archivo->puerto);
	if (socket_tmp < 0) {
		log_error_consola("No se pudo establecer conexion con el otro nodo para aparear");
		strcpy(resultado, "error.rmf");
		return resultado;
	}
	t_msg* msg = string_message(GET_NEXT_ROW, nodo_archivo->archivo, 0);
	enviar_mensaje(socket_tmp, msg);
	msg = recibir_mensaje(socket_tmp);
	if (msg) {
		if (msg->header.id == GET_NEXT_ROW_OK) {
			strcpy(resultado, msg->stream);
		}
		if (msg->header.id == GET_NEXT_ROW_ERROR) {
			log_error_consola("El nodo no devolvio el proximo registro. Devolvio ERROR.");
		}
	} else {
		log_error_consola("No se pudo establecer conexion con el otro nodo para aparear");
		strcpy(resultado, "error.rmf");
		return resultado;
	}
	return resultado;
}

void list_add_archivo_tmp(char* nombre_archivo) {
	t_archivo_tmp* archivo = malloc(sizeof(t_archivo_tmp));
	strcpy(archivo->nombre_archivo, nombre_archivo);
	archivo->posicion_puntero = NULL;
	list_add(archivos_temporales, archivo);
}

fpos_t* obtener_posicion_puntero_arch_tmp(char* nombre_archivo) {
	t_archivo_tmp* archivo;

	bool _archivo_con_nombre(t_archivo_tmp* archivo_tmp) {
		return string_equals_ignore_case(archivo_tmp->nombre_archivo, nombre_archivo);
	}
	archivo = list_find(archivos_temporales, (void*) _archivo_con_nombre);

	return archivo->posicion_puntero;
}

void actualizar_posicion_puntero_arch_tmp(char* nombre_archivo, fpos_t* posicion_puntero) {
	t_archivo_tmp* archivo;

	bool _archivo_con_nombre(t_archivo_tmp* archivo_tmp) {
		return string_equals_ignore_case(archivo_tmp->nombre_archivo, nombre_archivo);
	}

	archivo = list_find(archivos_temporales, (void*) _archivo_con_nombre);
	archivo->posicion_puntero = posicion_puntero;

}

