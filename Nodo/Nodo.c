/*
 * Nodo.c
 *
 *  Created on: 25/4/2015
 *      Author: utnso
 */
#include "Nodo.h"

int main() {


	Log_Nodo = log_create(LOG_FILE, PROCESO, 1, LOG_LEVEL_TRACE);

	if (levantarConfiguracionNodo()) {
		log_error(Log_Nodo, "Hubo errores en la carga de las configuraciones.");
	}

	_data = crear_Espacio_Datos(NODO_NUEVO, ARCHIVO_BIN,RUTA);

	//mapeo el disco en memoria
//	_data = file_get_mapped(DISCO);

	if (levantarHiloFile()) {
		log_error(Log_Nodo, "Conexion con File System fallida.");
	}

	if (levantarServer()) {
		log_error(Log_Nodo, "Error al levantar el Server.");
	}

	return 0;
}

int levantarConfiguracionNodo() {
 char* aux;
	t_config* archivo_config = config_create(PATH);

	PUERTO_FS = config_get_int_value(archivo_config, "PUERTO_FS");
	IP_FS = strdup(config_get_string_value(archivo_config, "IP_FS"));
	ARCHIVO_BIN = strdup(
			config_get_string_value(archivo_config, "ARCHIVO_BIN"));
	DIR_TEMP = strdup(config_get_string_value(archivo_config, "DIR_TEMP"));
	aux = strdup(config_get_string_value(archivo_config, "NODO_NUEVO"));
	IP_NODO = strdup(config_get_string_value(archivo_config, "IP_NODO"));
	PUERTO_NODO = config_get_int_value(archivo_config, "PUERTO_NODO");
    NODO_ID= PUERTO_FS = config_get_int_value(archivo_config, "NODO_ID");
	config_destroy(archivo_config);

	if (strncmp(aux,"SI",2)==0)
		 { NODO_NUEVO= 1;
				  }
	   else {NODO_NUEVO= 0;
				  }


	return 0;
}

int levantarHiloFile() {
	pthread_t thr_Conexion_FileSystem;
	t_conexion_nodo* reg_conexion = malloc(sizeof(t_conexion_nodo));

	reg_conexion->sock_fs = obtener_socket();
	rcx = pthread_create(&thr_Conexion_FileSystem, NULL,
			(void *) conectarFileSystem, reg_conexion);
	if (rcx != 0) {
		log_error(Log_Nodo,
				"El thread que acepta las conexiones entrantes no pudo ser creado.");
	}
	return 0;
}

void conectarFileSystem(t_conexion_nodo* reg_conexion) {
	conectar_socket(PUERTO_FS, IP_FS, reg_conexion->sock_fs);

	/*asegurarse de pasarle bien los parametros, puede recibir char*?*/
 t_msg* mensaje= argv_message(INFO_NODO,2,NODO_NUEVO,NODO_ID);
 enviar_mensaje(reg_conexion->sock_fs,mensaje);
 destroy_message(mensaje);

	t_msg*codigo = recibir_mensaje(reg_conexion->sock_fs);
	char*bloque= NULL ;
	t_msg* mensaje2;

	switch (codigo->header.id) {

	case GET_BLOQUE:
		//getbloque(bloque2,reg_conexion.sock_fs);
		/*											getBloque(numero) devovera el contenido del bloque "20*numero"
		 almacenado en el espacio de datos.
		 contenidoDeBloque getBloque(unNumero);
		 */

		bloque = getBloque(codigo->argv[0]);

		mensaje2 = string_message(GET_BLOQUE,bloque, 2, codigo->argv[0],
				tamanio_bloque);
		enviar_mensaje(reg_conexion->sock_fs, mensaje2);

		free_null((void*)&bloque);
		destroy_message(mensaje2);
		destroy_message(codigo);
		break;

	case SET_BLOQUE:
		/*
		 setBloque almacenara los "datos" en "20*numero"
		 setBloque(numero,datos);
		 */

		bloque= malloc(tamanio_bloque);
		memcpy(bloque, codigo->stream, codigo->argv[1]);//1 es el tamaño real, el stream es el bloque de 20mb(aprox)
		memset(bloque + codigo->argv[1], '\0',
				tamanio_bloque - codigo->argv[1]);
		setBloque(codigo->argv[0], bloque);

		free_null((void*)&bloque);
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
		mensaje2->stream=bloque;

		enviar_mensaje(reg_conexion->sock_fs, mensaje2);
		destroy_message(mensaje2);
		destroy_message(codigo);
		free_null((void*)&bloque);
		break;
	}

}



int levantarServer() {

	fd_set read_fs; // descriptores q estan lisots para leer
	fd_set master; //descriptores q q estan actualemnte conectados
	size_t tamanio; // hace positivo a la variable
	int socketEscucha, socketNuevaConexion;

	int max;
	struct sockaddr_in cliente;

	FD_ZERO(&master);
	FD_ZERO(&read_fs);

	socketEscucha = obtener_socket();
	vincular_socket(socketEscucha, PUERTO_NODO);
	if (listen(socketEscucha, 10) != 0) {
		perror("Error al poner a escuchar socket");
		return EXIT_FAILURE;
	}

	printf("Escuchando conexiones entrantes.\n");
// Escuchar nuevas conexiones entrantes.
//arranca select

	FD_SET(socketEscucha, &master);
	max = socketEscucha;
	printf("%d \n", socketEscucha);

	while (1) {

		memcpy(&read_fs, &master, sizeof(master));
		int dev_select;
		if ((dev_select = select(max + 1, &read_fs, NULL, NULL, NULL)) == -1) {
			perror("select");

		}
		//printf("select = %d \n",dev_select);
		int i;
		for (i = 0; i <= max; i++) //max : cantidad max de sockets
				{
			if (FD_ISSET(i, &read_fs)) {
				//  printf("i = %d \n max = %d \n",i,max);
				if (i == socketEscucha) {
					// pasar a una funcion generica aceptar();
					tamanio = sizeof(struct sockaddr_in);
					if ((socketNuevaConexion = accept(socketEscucha,
							(struct sockaddr*) &cliente, &tamanio)) < 0) {
						perror("Error al aceptar conexion entrante");
						return EXIT_FAILURE;
					} else {
						if (socketNuevaConexion > max) {
							max = socketNuevaConexion;
						}
						FD_SET(socketNuevaConexion, &master);
						//printf("nueva conexion de %s desde socket %d \n",inet_ntoa(cliente.sin_addr), socketNuevaConexion);
					} //if del accept. Recibir hasta BUFF_SIZE datos y almacenarlos en 'buffer'.
				} else {
//
					t_msg*codigo = recibir_mensaje(i);
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

						mensaje2 = string_message(GET_BLOQUE, bloque, 2,
								codigo->argv[0], tamanio_bloque);
						enviar_mensaje(i, mensaje2);

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

						enviar_mensaje(i, mensaje2);
						destroy_message(mensaje2);
						destroy_message(codigo);
						free_null((void*) &bloque);
						break;

					case EJECUTAR_MAP:
						mensaje2=recibir_mensaje(i);

						puts("voy a ejecutar map");
						/*
						 ejecutar_mapping(ejecutable,num_bloque,rchivo);
						 */

           ejecutar_map(codigo->stream,codigo->argv[0],mensaje2->stream);

						break;
					case EJECUTAR_REDUCE:
						printf("ejecutar_Reduce()");
						/*
						 ejecutar_reduce(ejecutable,lista_archivos,nombre_archivo_tmp);
						 */
						break;
					}
						fflush(stdout);


					}
				}						//1er if
			}						// for
		}


	close(socketEscucha);
	return EXIT_SUCCESS;

}

void setBloque(int numeroBloque, char* bloque_datos){
//debo pararme en la posicion donde se encuentra almacenado el bloque y empezar a grabar

	log_info(Log_Nodo, "Inicio setBloque(%d)", numeroBloque);

    //el memset lo hago para limpiar el bloque por las dudas
	memset(_data + (numeroBloque * tamanio_bloque), 0, tamanio_bloque);

	memcpy(_data + (numeroBloque * tamanio_bloque), bloque_datos, tamanio_bloque);

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

char* crear_Espacio_Datos(int NUEVO,char* ARCHIVO,char* RUT){
	size_t tamanio=1073741824;
	char* direccion;
	char* path;
	if(NUEVO==1) {
		path = file_combine(RUT, ARCHIVO);
		create_file(path,tamanio);
		direccion=file_get_mapped(path);

	} else {direccion = file_combine(RUT, ARCHIVO);

	}
	return direccion;
}
void ejecutar_map(char*ejecutable,int numeroBloque,char* nombreArchivo){
log_info(Log_Nodo, "Ini ejecutarMap en el bloque(%d)", numeroBloque);
log_info(Log_Nodo, "Fin ejecutarMap en el bloque(%d)", numeroBloque);
}
