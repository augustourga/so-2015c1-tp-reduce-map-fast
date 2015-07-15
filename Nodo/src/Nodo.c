#include "Nodo.h"

int main(int argc, char*argv[]) {

	log_crear("INFO", LOG_FILE, PROCESO);

	if (levantarConfiguracionNodo(argv[1])) {
		log_error_consola("Hubo errores en la carga de las configuraciones.");
		exit(1);
	}

	_data = levantar_espacio_datos();
	archivos_temporales = list_create();

//	if (levantarHiloFile()) {
//		log_error_consola("Conexion con File System fallida.");
//		exit(1);
//	}

	levantarNuevoServer();

	liberar_Espacio_datos(_data, ARCHIVO_BIN);

	return 0;
}

int levantarConfiguracionNodo(char* path) {

	t_config* archivo_config = config_create(path);

	PUERTO_FS = config_get_int_value(archivo_config, "PUERTO_FS");
	IP_FS = strdup(config_get_string_value(archivo_config, "IP_FS"));
	ARCHIVO_BIN = strdup(config_get_string_value(archivo_config, "ARCHIVO_BIN"));
	DIR_TEMP = strdup(config_get_string_value(archivo_config, "DIR_TEMP"));
	PUERTO_NODO = config_get_int_value(archivo_config, "PUERTO_NODO");
	NOMBRE_NODO = strdup(config_get_string_value(archivo_config, "NOMBRE_NODO"));
	config_destroy(archivo_config);

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

	fd_set read_fds;    // master file descriptor list
	int socket_actual;
	int fdmax;
	int socket_fs = reg_conexion->sock_fs;
	int res = 0;

	socket_fs = client_socket(IP_FS, PUERTO_FS);
	t_msg* mensaje = string_message(CONEXION_NODO, NOMBRE_NODO, 2, CANT_BLOQUES, PUERTO_NODO);

	res = enviar_mensaje(socket_fs, mensaje);
	if (res == -1) {
		log_error_consola("Fallo envio mensaje CONEXION_NODO");

		exit(1);
	}

	destroy_message(mensaje);
	log_info_consola("Conectado al File System en el socket %d", socket_fs);

	FD_ZERO(&read_fds);
	FD_SET(socket_fs, &read_fds);
	fdmax = socket_fs;

	while (true) {
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			log_error_consola("Falló el select");
			perror("Falló el select. Error");
			exit(1);
		}

		char*bloque = NULL;
		t_msg* mensaje2;
		t_msg*codigo;
		for (socket_actual = 0; socket_actual <= fdmax; socket_actual++) {
			if (FD_ISSET(socket_actual, &read_fds)) {
				if (socket_actual == socket_fs) {
					if ((codigo = recibir_mensaje(socket_fs)) != NULL) {

						switch (codigo->header.id) {

						case GET_BLOQUE:

							bloque = getBloque(codigo->argv[0]);
							mensaje2 = string_message(GET_BLOQUE_OK, bloque, 1, codigo->argv[1]);
							res = enviar_mensaje(socket_fs, mensaje2);
							if (res == -1) {
								log_error_consola("Fallo envio mensaje GET_BLOQUE_OK");
							}

							free(bloque);
							destroy_message(mensaje2);
							destroy_message(codigo);
							break;

						case SET_BLOQUE:

							if (codigo->argv[0] < CANT_BLOQUES) {
								bloque = malloc(tamanio_bloque);
								memcpy(bloque, codigo->stream, codigo->argv[1]); //1 es el tamaño real, el stream es el bloque de 20mb(aprox)
								memset(bloque + codigo->argv[1], '\0', tamanio_bloque - codigo->argv[1]);
								setBloque(codigo->argv[0], bloque);
								mensaje2 = id_message(SET_BLOQUE_OK);

								free(bloque);
							} else {
								mensaje2 = id_message(SET_BLOQUE_ERROR);
							}
							res = enviar_mensaje(socket_fs, mensaje2);
							if (res == -1) {
								log_error_consola("Fallo envio mensaje SET_BLOQUE");
							}

							destroy_message(codigo);
							break;

						case GET_FILE_CONTENT:

							bloque = getFileContent(codigo->stream);
							t_msg* resppuesta = string_message(GET_FILE_CONTENT_OK, bloque, 0);
							res = enviar_mensaje(socket_fs, resppuesta);
							if (res == -1) {
								log_error_consola("Fallo envio mensaje GET_FILE_CONTENT");
							}
							destroy_message(resppuesta);
							destroy_message(codigo);
							free(bloque);
							break;
						default:
							log_error_consola("Mensaje Incorrecto. Se esperaba GET_FILE_CONTEN | SET_BLOQUE | GET_BLOQUE");
							break;
						}
					} else {
						log_error_consola("Se ha desconectado el File System");
						shutdown(socket_actual, 2);
						close(socket_actual);
						exit(1);
					}
				}
			}
		}
	}
}

void levantarNuevoServer() {
	pthread_t thread;
	int listener, nuevaConexion;
	fd_set read_fds;  // temp file descriptor list for select()
	int fdmax;        // maximum file descriptor number
	int socket_actual;

	FD_ZERO(&read_fds);

	listener = server_socket(PUERTO_NODO);

	// add the listener to the master set
	FD_SET(listener, &read_fds);

	// keep track of the biggest file descriptor
	fdmax = listener; // so far, it's this one

	log_info_consola("Escuchando conexiones entrantes");

	while (true) {
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			log_error_consola("Falló el select");
			perror("Falló el select. Error");
			exit(1);
		}

		for (socket_actual = 0; socket_actual <= fdmax; socket_actual++) {
			if (FD_ISSET(socket_actual, &read_fds)) {
				if (socket_actual == listener) {
					nuevaConexion = accept_connection(listener);
					if (nuevaConexion >= 0) {
						log_info_consola("Se ha conectado un proceso al socket %d", nuevaConexion);
						int* parametro = malloc(sizeof(int));
						(*parametro) = nuevaConexion;
						pthread_create(&thread, NULL, (void*) atenderConexiones, (void*) parametro);
					}
				}
			}
		}

	}
}

void atenderConexiones(void *parametro) {
	t_msg *codigo;
	int sock_conexion = *((int *) parametro);
	char*bloque = NULL;
	t_msg* mensaje2;
	t_queue *cola_nodos = queue_create();
	t_msg_id fin;
	t_nodo_archivo* nodo_arch;
	char*path_rutina;
	int band = 0;
	int res = 0;

	if ((codigo = recibir_mensaje(sock_conexion)) != NULL) {

		switch (codigo->header.id) {

		case GET_BLOQUE:
			bloque = getBloque(codigo->argv[0]);
			mensaje2 = string_message(GET_BLOQUE_OK, bloque, 1, codigo->argv[1]);
			res = enviar_mensaje(sock_conexion, mensaje2);
			if (res == -1) {
				log_error_consola("Fallo envio mensaje GET_BLOQUE_OK");
			}

			free(bloque);
			destroy_message(mensaje2);
			destroy_message(codigo);
			break;

		case SET_BLOQUE:
			if (codigo->argv[0] < CANT_BLOQUES) {
				bloque = malloc(tamanio_bloque);
				memcpy(bloque, codigo->stream, codigo->argv[1]); //1 es el tamaño real, el stream es el bloque de 20mb(aprox)
				memset(bloque + codigo->argv[1], '\0', tamanio_bloque - codigo->argv[1]);
				setBloque(codigo->argv[0], bloque);
				mensaje2 = id_message(SET_BLOQUE_OK);

				free(bloque);
			} else {
				mensaje2 = id_message(SET_BLOQUE_ERROR);
			}
			res = enviar_mensaje(sock_conexion, mensaje2);
			if (res == -1) {
				log_error_consola("Fallo envio mensaje SET_BLOQUE");
			}

			destroy_message(codigo);
			break;

		case GET_FILE_CONTENT:

			bloque = getFileContent(codigo->stream);

			mensaje2 = id_message(GET_FILE_CONTENT);
			mensaje2->stream = bloque;

			res = enviar_mensaje(sock_conexion, mensaje2);
			if (res == -1) {
				log_error_consola("Fallo envio mensaje GET_FILE_CONTENT");
			}
			destroy_message(mensaje2);
			destroy_message(codigo);
			free(bloque);
			break;

		case EJECUTAR_MAP:

			mensaje2 = recibir_mensaje(sock_conexion);
			if (mensaje2->header.id == RUTINA) {
				char* path_rutina = guardar_rutina(mensaje2->stream,"map",mensaje2->argv[1], mensaje2->argv[0],codigo->argv[0]);
				fin = ejecutar_map(path_rutina, codigo->stream, codigo->argv[0], mensaje2->argv[0]);
				switch (fin) {
				case FIN_MAP_ERROR:
					log_error_consola("Hubo errores en el map %d en el bloque %d.", mensaje2->argv[0], codigo->argv[0]);
					break;
				case FIN_MAP_OK:
					log_info_consola("Map %d en el bloque %d exitoso", mensaje2->argv[0], codigo->argv[0]);
					break;
				default:
					log_info_consola("Mensaje incorrecto se esperaba el id FIN_MAP_OK ó FIN_MAP_ERROR");
					break;

				}
				mensaje2 = id_message(fin);
				res = enviar_mensaje(sock_conexion, mensaje2);
				if (res == -1) {
					log_error_consola("Fallo envio mensaje FIN_MAP");
				}
			} else {
				log_error_consola("Fallo en Recibir Rutina. Se esperaba el id RUTINA.");
			}

			destroy_message(mensaje2);
			destroy_message(codigo);

			break;

		case EJECUTAR_REDUCE:

			//Se recibe la rutina

			mensaje2 = recibir_mensaje(sock_conexion);

			if (mensaje2->header.id == RUTINA) {
				path_rutina = guardar_rutina(mensaje2->stream,"reduce",mensaje2->argv[0], codigo->argv[0],667);



				mensaje2 = recibir_mensaje(sock_conexion);

				while (mensaje2->header.id != FIN_ENVIO_MENSAJE) {

					if (mensaje2 != NULL) {
						nodo_arch = (t_nodo_archivo*) malloc(sizeof(t_nodo_archivo));
						nodo_arch->ip = string_n_split(mensaje2->stream, 3, "|")[0];
						nodo_arch->nombre = string_n_split(mensaje2->stream, 3, "|")[1];
						nodo_arch->archivo = string_n_split(mensaje2->stream, 3, "|")[2];
						nodo_arch->puerto = mensaje2->argv[0];
						queue_push(cola_nodos, (void*) nodo_arch);

						mensaje2 = recibir_mensaje(sock_conexion);

					} else {
						log_error_consola("Fallo en Recibir Nodos_archivos.");
						band = -1;
						break;
					}

				}
				if (band == 0) {
					fin = ejecutar_reduce(path_rutina, codigo->stream, cola_nodos, codigo->argv[0]);
					mensaje2 = id_message(fin);
					res = enviar_mensaje(sock_conexion, mensaje2);
					if (res == -1) {
						log_error_consola("Fallo envio mensaje FIN_REDUCE");
					}
				} else {
					log_error_consola("No se puede ejecutar Reduce, ya que no se han recibido correctamente los archivos ");
				}


			} else {
				log_error_consola("Fallo en Recibir Rutina.Se esperaba el id RUTINA.");
			}
			remove(path_rutina);
			destroy_message(mensaje2);
			destroy_message(codigo);
			break;

		case GET_NEXT_ROW:
			bloque = obtener_proximo_registro_de_archivo(codigo->stream);
			if (bloque != NULL) {
				mensaje2 = string_message(GET_NEXT_ROW_OK, bloque, 0);
				log_info_consola("GET_NEXT_ROW_OK");

			} else {
				mensaje2 = id_message(GET_NEXT_ROW_ERROR);
				log_info_consola("GET_NEXT_ROW_ERROR");
			}
			res = enviar_mensaje(sock_conexion, mensaje2);
			if (res == -1) {
				log_error_consola("Fallo envio mensaje GET_BLOQUE_OK");
			}
			destroy_message(mensaje2);
			destroy_message(codigo);

			break;
		default:
			log_info_consola("Mensaje incorrecto.Se esperaba GET_BLOQUE|SET_BLOQUE|GET_FILE_CONTENT| EJECUTAR_MAP| EJECUTAR_REDUCE | GET_NEXT_ROW");
			break;
		}

	} else {
		log_error_consola("Se ha desconectado un proceso del socket %d", sock_conexion);

	}
	shutdown(sock_conexion, 2);
	close(sock_conexion);
}

void setBloque(int numeroBloque, char* bloque_datos) {
	//debo pararme en la posicion donde se encuentra almacenado el bloque y empezar a grabar
	log_info_consola("Inicio setBloque(%d)", numeroBloque);
	memcpy(_data + (numeroBloque * tamanio_bloque), bloque_datos, tamanio_bloque);
	log_info_consola("Fin setBloque(%d)", numeroBloque);

}

char* getBloque(int numeroBloque) {
	log_info_consola("Ini getBloque(%d)", numeroBloque);
	char* bloque = NULL;
	bloque = malloc(tamanio_bloque);
	memcpy(bloque, &(_data[numeroBloque * tamanio_bloque]), tamanio_bloque);
	log_info_consola("Fin getBloque(%d)", numeroBloque);
	return bloque;
}

char* getFileContent(char* filename) {

	log_info_consola("Inicio getFileContent(%s)", filename);
	char* content = NULL;
	//creo el espacio para almacenar el archivo
	char* path = file_combine(DIR_TEMP, filename);
	/*size_t size = file_get_size(path) + 1;
	content = malloc(size);
	char* mapped = NULL;
	mapped = file_get_mapped(path);
	memcpy(content, mapped, size);        //
	file_mmap_free(mapped, path);*/
	content = read_whole_file(path);
	log_info_consola("Fin getFileContent(%s)", path);
	return content;

}

char* levantar_espacio_datos() {
	CANT_BLOQUES = file_get_size(ARCHIVO_BIN) / tamanio_bloque;
	log_info_consola("Levantado %s. Cantidad de bloque:%d ", ARCHIVO_BIN, CANT_BLOQUES);

	if(!file_exists(DIR_TEMP)){
    int res=mkdir(DIR_TEMP,0777);
    if (!res){log_info_consola("Directorio %s creado correctamente ",DIR_TEMP);

    }else{
    	log_error_consola("Directorio %s no pudo ser creado ",DIR_TEMP);
    exit(1);
    }

     }

	return file_get_mapped(ARCHIVO_BIN);

}

void liberar_Espacio_datos(char* _data, char* path) {

	file_mmap_free(_data, path);
}

t_msg_id ejecutar_map(char*path_ejecutable, char* nombreArchivo, int numeroBloque, int mapid) {
	log_info_consola("Inicio ejecutarMap ID:%d en el bloque %d", mapid, numeroBloque);
	char*bloque = NULL;
	bloque = getBloque(numeroBloque);

	char*nombreArchivoFinal = file_combine(DIR_TEMP, nombreArchivo);
	log_info_consola("Fin copia de ejecutable ID:%d en el bloque %d", mapid, numeroBloque);
	if (ejecuta_map(bloque, path_ejecutable, nombreArchivoFinal)) {
		return FIN_MAP_ERROR;
	}

	log_info_consola("Fin rutina de map ID:%d en el bloque %d", mapid, numeroBloque);

	list_add_archivo_tmp(nombreArchivoFinal);
	remove(path_ejecutable);

	log_info_consola("Fin ejecutarMap ID:%d en el bloque %d", mapid, numeroBloque);
	return FIN_MAP_OK;

}

t_msg_id ejecutar_reduce(char*path_ejecutable, char* nombreArchivoFinal, t_queue* colaArchivos, int id_reduce) {

	log_info_consola("Inicio ejecutar Reduce ID:%d ", id_reduce);
	t_list* lista_nodos;
	list_add_archivo_tmp("archivo2.txt");
	list_add_archivo_tmp("archivo3.txt");
	char* path_final = file_combine(DIR_TEMP, nombreArchivoFinal);

	char* nombretemporal = generar_nombre_temporal(id_reduce, "reduce", 667);
	char* temporal = file_combine(DIR_TEMP, nombretemporal);

	lista_nodos = deserealizar_cola(colaArchivos);
	int res;
	res = apareo(temporal, lista_nodos, path_ejecutable, path_final);
	if (res == -1) {
		//remove(temporal);
		return FIN_REDUCE_ERROR;
	}

	remove(temporal);
	list_add_archivo_tmp(nombreArchivoFinal);
	log_info_consola("Fin ejecutar Reduce ID:%d ", id_reduce);
	return FIN_REDUCE_OK;
}

t_list* deserealizar_cola(t_queue* colaArchivos) {
	int k, a;
	char** lista_archivos_aux;
	t_list* lista_nodos = list_create();
	t_nodo_archivo* elem = (t_nodo_archivo*) malloc(sizeof(t_nodo_archivo));
	a = queue_size(colaArchivos);
	elem = (t_nodo_archivo *) queue_pop(colaArchivos);
	while (a != 0) {
		lista_archivos_aux = string_split(elem->archivo, ";");
		for (k = 0; lista_archivos_aux[k] != NULL; k++) {
			t_nodo_archivo* nodo_aux = (t_nodo_archivo*) malloc(sizeof(t_nodo_archivo));
			nodo_aux->ip = elem->ip;
			nodo_aux->puerto = elem->puerto;
			nodo_aux->archivo = lista_archivos_aux[k];
			nodo_aux->nombre = elem->nombre;
			nodo_aux->numero_linea = 0;
			list_add(lista_nodos, nodo_aux);
		}
		a = queue_size(colaArchivos);
		elem = (t_nodo_archivo *) queue_pop(colaArchivos);

	}
	free(elem);
	return lista_nodos;
}
char* guardar_rutina(char* ejecutable,char* map_o_reduce,size_t tamanio,int mapid,int numeroBloque){
	char* nombre_rutina = generar_nombre_rutina(mapid, map_o_reduce, numeroBloque);
		char* path_ejecutable = file_combine(DIR_TEMP, nombre_rutina);
		write_file(path_ejecutable, ejecutable, tamanio);
		chmod(path_ejecutable, S_IRWXU);
return path_ejecutable;
}

int apareo(char* temporal, t_list* lista_nodos_archivos, char* path_ejecutable, char* path_salida) {
	char** registros = malloc(sizeof(int));
	int i, pos;
	int res = 0;
	char* registro_actual;
	char* aux_string;

	int in[2];
	if (pipe(in) < 0) {
		error("pipe in");
		return -1;
	}

	pid_t pid = ejecuta_reduce(in, path_ejecutable, path_salida);

	close(in[0]);

	FILE* archivo = fopen(temporal, "a+");

	int cantidad_nodos_archivos = list_size(lista_nodos_archivos);
	t_nodo_archivo* elem = (t_nodo_archivo*) malloc(sizeof(t_nodo_archivo));

	// Aca se fija si es un solo archivo para aparear o si son varios
	if (cantidad_nodos_archivos == 1) {
		elem = (t_nodo_archivo *) list_get(lista_nodos_archivos, 0);

		char* archivo = read_whole_file(elem->archivo);

		write(in[1], archivo, strlen(archivo));

		free(archivo);
	} else {
// Obtengo el primer registro de cada archivo
		for (i = 0; i < cantidad_nodos_archivos; i++) {
			elem = (t_nodo_archivo *) list_get(lista_nodos_archivos, i);
			aux_string = obtener_proximo_registro(elem);
			if (string_equals_ignore_case(aux_string, "error.rmf.GrupoMilanesa.tp")) {
				res = -1;
				return res;
			} else {
				registros[i] = aux_string;
			}
		}

		pos = obtener_posicion_menor_clave(registros, cantidad_nodos_archivos);
// obtener_posicion_menor_clave devuelve -1 una vez que en el array ya son todos campos nulos u EOF
		while (pos != -1) {
			registro_actual = registros[pos];
			elem = (t_nodo_archivo *) list_get(lista_nodos_archivos, pos);
			aux_string = obtener_proximo_registro(elem);
			if (aux_string == NULL) {
				registros[pos] = NULL;
			} else {
				if (string_equals_ignore_case(aux_string, "error.rmf.GrupoMilanesa.tp")) {
					res = -1;
					return res;
				} else {
					registros[pos] = aux_string;
				}
			}
			if (archivo != NULL) {
				// TODO: Esto se cambiaria por un write para no usar el archivo temporal
				//log_info_consola("Escribo registro: %s en archivo: %s", registro_actual, temporal);
				//fputs(registro_actual, archivo);
				write(in[1], registro_actual, strlen(registro_actual));
			} else {
				log_error_consola("No se pudo acceder al archivo temporal para guardar data de apareamiento");
				res = -1;
				return res;
			}
			pos = obtener_posicion_menor_clave(registros, cantidad_nodos_archivos);
		}
	}

	close(in[1]);
//free_puntero_puntero(registros);
//fclose(archivo);
	int status;
	waitpid(pid, &status, 0);
	log_info_consola("Se aparearon correctamente los archivos.");
	return status;
	return res;
}

char* obtener_proximo_registro(t_nodo_archivo* nodo_archivo) {
	char* respuesta;
	char* resultado;

	if (nodo_archivo->numero_linea == 0) {
		if (string_equals_ignore_case(nodo_archivo->nombre, NOMBRE_NODO)) {
			respuesta = obtener_proximo_registro_de_archivo(nodo_archivo->archivo);
		} else {
			respuesta = enviar_mensaje_proximo_registro(nodo_archivo);
		}
		nodo_archivo->lineas = string_split(respuesta, "\n");
		free(respuesta);
	}
	char* linea = nodo_archivo->lineas[nodo_archivo->numero_linea];
	if (linea != NULL) {
		resultado = string_duplicate(linea);
		string_append(&resultado, "\n");
		nodo_archivo->numero_linea++;
		free(linea);
	} else {
		resultado = NULL;
		free(nodo_archivo->lineas);
	}

	return resultado;
}

int obtener_posicion_menor_clave(char** registros, int cantidad_nodos_archivos) {
	int pos, i;
	char* registro_1 = NULL;

// obtiene primer campo != NULL
	for (pos = 0; pos < cantidad_nodos_archivos; pos++) {
		if (registros[pos] != NULL) {
			registro_1 = registros[pos];
			break;
		}
// devuelve -1 una vez que en el array ya son todos campos nulos
	}
	if (registro_1 == NULL) {
		pos = -1;
	} else {
		for (i = 0; i < cantidad_nodos_archivos; i++) {
			if (!registros[i]) {
				continue;
			}
			if (strcmp(registro_1, registros[i]) > 0) {
				pos = i;
				registro_1 = registros[i];
			}
		}
	}
	return pos;
}

char* obtener_proximo_registro_de_archivo(char* archivo) {
	char* path = file_combine(DIR_TEMP, archivo);
	/*fpos_t* posicion_puntero = obtener_posicion_puntero_arch_tmp(path);
	 int bytes_read;
	 size_t buffer_size = 100;
	 char* linea = (char *) calloc(1, buffer_size);

	 FILE* file = fopen(path, "r");
	 if (file != NULL) {
	 if (posicion_puntero != NULL) {
	 fsetpos(file, posicion_puntero);
	 }
	 bytes_read = getline(&linea, &buffer_size, file);
	 if (bytes_read == -1) {
	 linea = NULL;
	 } else {
	 posicion_puntero = malloc(sizeof(fpos_t));
	 fgetpos(file, posicion_puntero);
	 actualizar_posicion_puntero_arch_tmp(path, posicion_puntero);
	 log_debug_consola("Obtuve registro: %s del archivo: %s", linea, path);
	 }
	 fclose(file);
	 } else {
	 log_error_consola("No pudo abrirse el archivo temporal");
	 linea = NULL;
	 }
	 return linea;*/
	return read_whole_file(path);
}

char* enviar_mensaje_proximo_registro(t_nodo_archivo* nodo_archivo) {
	char* resultado = string_new();
	int socket_tmp;
	int res = 0;
	socket_tmp = client_socket(nodo_archivo->ip, nodo_archivo->puerto);
	if (socket_tmp < 0) {
		log_error_consola("No se pudo establecer conexion con el otro nodo para aparear");
		free(resultado);
		resultado = string_duplicate("error.rmf.GrupoMilanesa.tp");
		return resultado;
	}
	t_msg* msg = string_message(GET_NEXT_ROW, nodo_archivo->archivo, 0);
	res = enviar_mensaje(socket_tmp, msg);
	if (res == -1) {
		log_error_consola("Fallo envio mensaje GET_NEXT_ROW");
	}
	msg = recibir_mensaje(socket_tmp);
	if (msg) {
		if (msg->header.id == GET_NEXT_ROW_OK) {
			resultado = string_duplicate(msg->stream);
		}
		if (msg->header.id == GET_NEXT_ROW_ERROR) {
			log_error_consola("El nodo no devolvio el proximo registro. Devolvio ERROR.");
		}
	} else {
		log_error_consola("No se pudo establecer conexion con el otro nodo para aparear");
		resultado = string_duplicate("error.rmf.GrupoMilanesa.tp");
		return resultado;
	}
	destroy_message(msg);
	return resultado;
}

void list_add_archivo_tmp(char* nombre_archivo) {
	t_archivo_tmp* archivo = malloc(sizeof(t_archivo_tmp));
	archivo->nombre_archivo = nombre_archivo;
	archivo->posicion_puntero = NULL;
	list_add(archivos_temporales, archivo);
}

fpos_t* obtener_posicion_puntero_arch_tmp(char* nombre_archivo) {
	t_archivo_tmp* archivo;

	bool _archivo_con_nombre(t_archivo_tmp* archivo_tmp) {
		return (strcmp(archivo_tmp->nombre_archivo, nombre_archivo) == 0);
	}
	archivo = list_find(archivos_temporales, (void*) _archivo_con_nombre);

	return archivo->posicion_puntero;
}

void actualizar_posicion_puntero_arch_tmp(char* nombre_archivo, fpos_t* posicion_puntero) {
	t_archivo_tmp* archivo;

	bool _archivo_con_nombre(t_archivo_tmp* archivo_tmp) {
		return (strcmp(archivo_tmp->nombre_archivo, nombre_archivo) == 0);
	}

	archivo = list_find(archivos_temporales, (void*) _archivo_con_nombre);
	archivo->posicion_puntero = posicion_puntero;
}

