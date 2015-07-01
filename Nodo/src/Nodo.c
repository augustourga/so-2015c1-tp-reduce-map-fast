/*
 * Nodo.c
 *
 *  Created on: 25/4/2015
 *      Author: utnso
 */
#include "Nodo.h"

int main(int argc,char*parametros[]) {


	Log_Nodo = log_create(LOG_FILE, PROCESO, 1, LOG_LEVEL_TRACE);

	if (levantarConfiguracionNodo()) {
		log_error(Log_Nodo, "Hubo errores en la carga de las configuraciones.");
	}

	_data = crear_Espacio_Datos(NODO_NUEVO, ARCHIVO_BIN,parametros[0]);

	if (levantarHiloFile()) {
		log_error(Log_Nodo, "Conexion con File System fallida.");
	}

	levantarNuevoServer();

	return 0;
}

int levantarConfiguracionNodo() {
	char* aux;
	t_config* archivo_config = config_create(PATH_CONFIG);

	PUERTO_FS = config_get_int_value(archivo_config, "PUERTO_FS");
	IP_FS = strdup(config_get_string_value(archivo_config, "IP_FS"));
	ARCHIVO_BIN = strdup(config_get_string_value(archivo_config, "ARCHIVO_BIN"));
	DIR_TEMP = strdup(config_get_string_value(archivo_config, "DIR_TEMP"));
	aux = strdup(config_get_string_value(archivo_config, "NODO_NUEVO"));
	PUERTO_NODO = config_get_int_value(archivo_config, "PUERTO_NODO");
	NOMBRE_NODO=strdup(config_get_string_value(archivo_config, "NOMBRE_NODO"));
	config_destroy(archivo_config);


	if (strncmp(aux, "SI", 2) == 0) {
		NODO_NUEVO = 1;
	} else {
		NODO_NUEVO = 0;
	}


	return 0;
}

int levantarHiloFile() {
	pthread_t thr_Conexion_FileSystem;
	t_conexion_nodo* reg_conexion = malloc(sizeof(t_conexion_nodo));


	rcx = pthread_create(&thr_Conexion_FileSystem, NULL,
			(void *) conectarFileSystem, reg_conexion);
	if (rcx != 0) {
		log_error(Log_Nodo,
				"El thread que acepta las conexiones entrantes no pudo ser creado.");
	}
	return 0;
}

void conectarFileSystem(t_conexion_nodo* reg_conexion) {

	reg_conexion->sock_fs = client_socket(IP_FS, PUERTO_FS);
	t_msg* mensaje = string_message(CONEXION_NODO, NOMBRE_NODO,1, CANT_BLOQUES);
	enviar_mensaje(reg_conexion->sock_fs, mensaje);
	destroy_message(mensaje);

	while (true) {

		t_msg*codigo = recibir_mensaje(reg_conexion->sock_fs);
		char*bloque = NULL;
		t_msg* mensaje2;

		switch (codigo->header.id) {

		case GET_BLOQUE:
			//getbloque(bloque2,reg_conexion.sock_fs);
			/*											getBloque(numero) devovera el contenido del bloque "20*numero"
			 almacenado en el espacio de datos.
			 contenidoDeBloque getBloque(unNumero);
			 */
		    bloque = getBloque(codigo->argv[0]);
			mensaje2 = string_message(GET_BLOQUE, bloque, 2, codigo->argv[0],
					tamanio_bloque);
			enviar_mensaje(reg_conexion->sock_fs, mensaje2);
			free_null((void*) &bloque);
			destroy_message(mensaje2);
			destroy_message(codigo);
			break;

		case SET_BLOQUE:
			/*
			 setBloque almacenara los "datos" en "20*numero"
			 setBloque(numero,datos);
			 */
			bloque = malloc(tamanio_bloque);
			memcpy(bloque, codigo->stream, codigo->argv[1]); //1 es el tamaño real, el stream es el bloque de 20mb(aprox)
			memset(bloque + codigo->argv[1], '\0',
					tamanio_bloque - codigo->argv[1]);
			setBloque(codigo->argv[0], bloque);
			free_null((void*) &bloque);
			destroy_message(codigo);
			break;

		case GET_FILE_CONTENT:
			/*
			 arch getFileContent(char* nombre) devolvera el contenido
			 * del archivo "nombre.dat" almacenado en el espacio temporal
			 getFileContent(nombre);
			 */
			bloque = getFileContent(codigo->stream);
			mensaje2 = id_message(GET_FILE_CONTENT);
			mensaje2->stream = bloque;
			enviar_mensaje(reg_conexion->sock_fs, mensaje2);
			destroy_message(mensaje2);
			destroy_message(codigo);
			free_null((void*) &bloque);
			break;
		}

	}
}

void levantarNuevoServer() {
	pthread_t thread;
	int listener, nuevaConexion;
	listener = server_socket(PUERTO_NODO);
	printf("Esperando conexiones entrantes\n");
	while (true) {
		nuevaConexion = accept_connection(listener);
		pthread_create(&thread, NULL, atenderConexiones, &nuevaConexion);
	}
}

void *atenderConexiones(void *parametro) {
	t_msg *codigo;
	int sock_conexion = *((int *) parametro);
	char*bloque = NULL;
	t_msg* mensaje2;
	t_list* lista_nodos;
	t_msg_id fin;

	while (1) {
		if ((codigo = recibir_mensaje(sock_conexion)) != NULL ) {

			switch (codigo->header.id) {

			case GET_BLOQUE:
				//getbloque(bloque2,reg_conexion.sock_fs);
				/*											getBloque(numero) devovera el contenido del bloque "20*numero"
				 almacenado en el espacio de datos.
				 contenidoDeBloque getBloque(unNumero);
				 */
				bloque = getBloque(codigo->argv[0]);
				mensaje2 = string_message(GET_BLOQUE, bloque, 2,
						codigo->argv[0], tamanio_bloque);
				enviar_mensaje(sock_conexion, mensaje2);
				free_null((void*) &bloque);
				destroy_message(mensaje2);
				destroy_message(codigo);
				break;

			case SET_BLOQUE:
				/*
				 setBloque almacenara los "datos" en "20*numero"
				 setBloque(numero,datos);
				 */
				bloque = malloc(tamanio_bloque);
				memcpy(bloque, codigo->stream, codigo->argv[1]); //1 es el tamaño real, el stream es el bloque de 20mb(aprox)
				memset(bloque + codigo->argv[1], '\0',
						tamanio_bloque - codigo->argv[1]);
				setBloque(codigo->argv[0], bloque);
				free_null((void*) &bloque);
				destroy_message(codigo);
				break;

			case GET_FILE_CONTENT:
				/*
				 arch getFileContent(char* nombre) devolvera el contenido
				 * del archivo "nombre.dat" almacenado en el espacio temporal
				 getFileContent(nombre);
				 */
				bloque = getFileContent(codigo->stream);

				mensaje2 = id_message(GET_FILE_CONTENT);
				mensaje2->stream = bloque;

				enviar_mensaje(sock_conexion, mensaje2);
				destroy_message(mensaje2);
				destroy_message(codigo);
				free_null((void*) &bloque);
				break;

			case EJECUTAR_MAP:

				mensaje2 = recibir_mensaje(sock_conexion);
				puts("voy a ejecutar map");
				/* ejecutar_mapping(ejecutable,num_bloque,rchivo); */

				bloque = getBloque(codigo->argv[0]);
				char* temporal = generar_nombre_temporal(mensaje2->argv[0],
						"map", codigo->argv[0]);
				fin = ejecutar_map(mensaje2->stream, bloque, codigo->stream,
						temporal);
				switch (fin) {
				case FIN_MAP_ERROR:
					log_error(Log_Nodo,
							"Hubo errores en el map %d en el bloque %d.",
							mensaje2->argv[0], codigo->argv[0]);
					break;
				case FIN_MAP_OK:
					log_info(Log_Nodo, "Map %d en el bloque %d exitoso",
							mensaje2->argv[0], codigo->argv[0]);
					break;

				}
				mensaje2 = id_message(fin);
				enviar_mensaje(sock_conexion, mensaje2);
				destroy_message(mensaje2);
				destroy_message(codigo);
				free(bloque);
				break;

			case EJECUTAR_REDUCE:
//
				lista_nodos = list_create();
							t_nodo_archivo* nodo_arch;
							char* archivo_final= codigo->stream;
							char*rutina_reduce;
							//Se recibe la rutina
							mensaje2 = recibir_mensaje(sock_conexion);
							size_t tamanioEjecutable=strlen(mensaje2->stream);
							rutina_reduce = malloc(tamanioEjecutable);
							memcpy(rutina_reduce, mensaje2->stream, tamanioEjecutable);
							printf("ejecutar_Reduce()");
							destroy_message(mensaje2);

							//el primero va a ser el nodo local
							while(mensaje2->header.id!=FIN_ENVIO_MENSAJE){
								mensaje2 = recibir_mensaje(sock_conexion);
								nodo_arch=(t_nodo_archivo*) malloc(sizeof(t_nodo_archivo));
								nodo_arch->ip =string_n_split(mensaje2->stream,2,"|")[0];
								nodo_arch->archivos = string_n_split(mensaje2->stream,2,"|")[1];
								nodo_arch->puerto= mensaje2->argv[0];
								list_add(lista_nodos,(void*) nodo_arch);
							}
							/*
							 * EJECUTAR REDUCE
//							 */
		                    fin = ejecutar_reduce(rutina_reduce,archivo_final,lista_nodos);
							mensaje2=id_message(fin);
							enviar_mensaje(sock_conexion,mensaje2);
							destroy_message(mensaje2);
							destroy_message(codigo);
							free(rutina_reduce);
							break;

			}

		} else {
//			log_warning(Logger, "Desconexión de un proceso.");
			break;
		}
	}
	return NULL ;
}


void setBloque(int numeroBloque, char* bloque_datos) {
    //debo pararme en la posicion donde se encuentra almacenado el bloque y empezar a grabar
	log_info(Log_Nodo, "Inicio setBloque(%d)", numeroBloque);
	//el memset lo hago para limpiar el bloque por las dudas
	memset(_data + (numeroBloque * tamanio_bloque), 0, tamanio_bloque);
	memcpy(_data + (numeroBloque * tamanio_bloque), bloque_datos,
			tamanio_bloque);
	log_info(Log_Nodo, "Fin setBloque(%d)", numeroBloque);

}
char* getBloque(int numeroBloque) {
	log_info(Log_Nodo, "Ini getBloque(%d)", numeroBloque);
	char* bloque = NULL;
	bloque = malloc(tamanio_bloque);
	memcpy(bloque, &(_data[numeroBloque * tamanio_bloque]), tamanio_bloque);
	//memcpy(bloque, _bloques[numero], TAMANIO_BLOQUE);
	log_info(Log_Nodo, "Fin getBloque(%d)", numeroBloque);
	return bloque;
}


char* getFileContent(char* filename) {

       log_info(Log_Nodo, "Inicio getFileContent(%s)", filename);
       char* content = NULL;
       //creo el espacio para almacenar el archivo
       char* path = file_combine(DIR_TEMP, filename);
       size_t size = file_get_size(path) + 1;
       content = malloc(size);
       printf("size: %d\n", size);
       char* mapped = NULL;
       mapped = file_get_mapped(path);
       memcpy(content, mapped, size);        //
       file_mmap_free(mapped, path);
       free_null((void*)&path);
       log_info(Log_Nodo, "Fin getFileContent(%s)", filename);
       return content;

}

char* crear_Espacio_Datos(int NUEVO, char* ARCHIVO, char* parametro) {
	CANT_BLOQUES = atoi(parametro);
	size_t tamanio = 20 * 1024 * 1024;
	char* direccion;
	char* path;
	if (NUEVO == 1) {
		path = file_combine(DIR_TEMP, ARCHIVO);
		create_file(path, CANT_BLOQUES * tamanio);
		direccion = file_get_mapped(path);

	} else {
		path = file_combine(DIR_TEMP, ARCHIVO);
		direccion = file_get_mapped(path);
	}
	return direccion;
}

t_msg_id ejecutar_map(char*ejecutable, char* bloque, char* nombreArchivoFinal,
		char*temporal) {
	log_info(Log_Nodo, "Inicio ejecutarMap ");
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
	remove(path_ejecutable);
	remove(temporal);
	return FIN_MAP_OK;
	log_info(Log_Nodo, "Fin ejecutarMap ");
}


t_msg_id ejecutar_reduce(char*ejecutable,char*archivo_final,t_list* listaArchivos){
log_info(Log_Nodo, "Inicio ejecutarReduce " );

//LE PIDO AL SOCKET_NODO QUE ME DEVUELVA UN GET_FILE_CONTENT(nombreArchivo)
//una vez recibido, los apareo con mis archivos del espacio temporal

log_info(Log_Nodo, "Fin ejecutarReduce ");
return FIN_REDUCE_OK;
}


