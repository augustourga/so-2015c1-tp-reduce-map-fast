/*
 * FileSystem.c
 *
 *  Created on: 1/5/2015
 *      Author: tomasyagas
 */

#include "FileSystem.h"

int PUERTO_LISTEN;
unsigned int CANTIDAD_NODOS_MINIMA;
t_list* NODOS_CONECTADOS;
t_list* NODOS_ESPERANDO_CONEXION;
t_log* LOGGER;
bool STATUS;
unsigned int ESPACIO_LIBRE_TOTAL;
DB *dbHandler;

void levantar_configuracion() {
	t_config* config;
	config = config_create("config_file_system.cfg");
	if (config_has_property(config, "PUERTO_LISTEN")) {
		PUERTO_LISTEN = config_get_int_value(config, "PUERTO_LISTEN");
	}
	if (config_has_property(config, "CANTIDAD_NODOS_MINIMA")) {
		CANTIDAD_NODOS_MINIMA = config_get_int_value(config,
				"CANTIDAD_NODOS_MINIMA");
	}
	config_destroy(config);
}

void crear_logger() {
	LOGGER = log_create("log", "FileSystem", false, LOG_LEVEL_INFO);
}

void formatear_mdfs() {

}

void eliminar_renombrar_mover_archivos() {

}

void crear_eliminar_renombrar_mover_directorios() {

}

void copiar_archivo_local_al_mdfs() {

}

void copiar_archivo_mdfs_al_fs_local() {

}

void solicitar_md5_de_archivo_mdfs() {

}

void ver_borrar_copiar_bloques_de_archivo() {

}

void agregar_un_nodo() {
	if (list_size(NODOS_ESPERANDO_CONEXION)>0) {
		t_nodo *nodo = (t_nodo*) list_remove(NODOS_ESPERANDO_CONEXION, 0);
		nodo->cantidad_bloques_libres=50;
		nodo->conectado_andando=true;

		list_add(NODOS_CONECTADOS, nodo);
		ESPACIO_LIBRE_TOTAL = ESPACIO_LIBRE_TOTAL + 1024000; // 1000 MB (50 bloques de 20 MB)
	}
	if (!STATUS && CANTIDAD_NODOS_MINIMA <= list_size(NODOS_CONECTADOS)) {
		STATUS = true;
	}
}

void eliminar_un_nodo() {

}

void consola_file_system() {
	int valor = 1;
	while (valor) {
		int opcion;
		printf("Ingrese el numero de la opcion de lo que desea hacer:\n"
				"1 - Formatear el MDFS\n"
				"2 - Eliminar/Renombrar/Mover archivos\n"
				"3 - Crear/Eliminar/Renombrar/Mover directorios\n"
				"4 - Copiar un archivo local al MDFS\n"
				"5 - Copiar un archivo del MDFS al filesystem local\n"
				"6 - Solicitar el MD5 de un archivo en MDFS\n"
				"7 - Ver/Borrar/Copiar los bloques que componen un archivo\n"
				"8 - Agregar un nodo de datos\n"
				"9 - Eliminar un nodo de datos\n"
				"10 - Salir\n");
		scanf("%d", &opcion);
		switch (opcion) {
		case 1:
			formatear_mdfs();
			break;
		case 2:
			eliminar_renombrar_mover_archivos();
			break;
		case 3:
			crear_eliminar_renombrar_mover_directorios();
			break;
		case 4:
			copiar_archivo_local_al_mdfs();
			break;
		case 5:
			copiar_archivo_mdfs_al_fs_local();
			break;
		case 6:
			solicitar_md5_de_archivo_mdfs();
			break;
		case 7:
			ver_borrar_copiar_bloques_de_archivo();
			break;
		case 8:
			agregar_un_nodo();
			break;
		case 9:
			eliminar_un_nodo();
			break;
		case 10:
			valor = 0;
			break;
		default:
			printf("No ha ingresado una opcion valida\n");
		}
	}
}

void resend_msg(int new_fd, char *msg, int len, int flag, int bytes_sent) {
	int i = 0;
	while (len != bytes_sent && i < 6) {
		bytes_sent = send(new_fd, msg, len, 0);
		i++;
	}
	if (len != bytes_sent) {
		log_error(LOGGER, "No pudo ser enviado el mensaje por send():%s", msg);
	}
}

int agregar_nodo_nuevo(int id, int socket) {
	t_nodo *nodo = (t_nodo *) malloc(sizeof(t_nodo));
	nodo->id = id;
	nodo->socket = socket;
	int res;
	res = list_add(NODOS_ESPERANDO_CONEXION, nodo);
	if (res != 0) {
		log_error(LOGGER,
				"No se pudo agregar el nodo a la lista de nodos.");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void agregar_nodo_viejo(int id, int socket) {
	void* buscar_nodo(t_nodo* iterador){
	    return iterador->id == id;
	}
	t_nodo* nodo = (t_nodo*) list_find(NODOS_CONECTADOS, (bool(*)(void*))buscar_nodo);
	nodo->socket = socket;
	nodo->conectado_andando = true;
	ESPACIO_LIBRE_TOTAL = ESPACIO_LIBRE_TOTAL + ((nodo->cantidad_bloques_libres) * 20480);
}

void aceptar_conexion_nodo(t_bloque* bloque, int socket) {
	t_aceptacion_nodo* msg = deserializar_aceptacion_nodo(bloque);

	log_info(LOGGER, "NODO CONECTADO: %d\n", &msg->id_nodo);
	if (msg->nodo_nuevo) {
		agregar_nodo_nuevo(msg->id_nodo, socket);
	} else {
		agregar_nodo_viejo(msg->id_nodo, socket);
	}
}

void guardar_socket_marta(int socket) {

}

//void manejar_conexiones_nuevas(int socket) {
//	int length = 1024, bytes_recv;
//	char* buffer = 0;
//	bytes_recv = recv(socket, buffer, length, 0);
//	if (bytes_recv > 0) {
//		int offset = 0, tmp_size = 0, code;
//		tmp_size = sizeof(code);
//		memcpy(&code, buffer, tmp_size);
//		offset += tmp_size;
//		t_bloque* bloque = malloc(sizeof(t_bloque));
//		bloque->data = buffer + offset;
//		bloque->size = strlen(bloque->data);
//		switch (code) {
//		case 100:
//			aceptar_conexion_nodo(bloque, socket);
//			break;
//		case 200:
//			guardar_socket_marta(socket);
//			break;
//		}
//	} else {
//		printf("Laaaaaaa cooooncha !!! El recv devolvió: %d\n", bytes_recv);
//	}
//}
void conexiones_file_system() {
// Abre su puerto de escucha
	int backLog = 20; // Número de conexiones permitidas en la cola de entrada (hasta que se aceptan)
	int socketEscucha;
	socketEscucha = crear_socket_listen(PUERTO_LISTEN);

	if (listen(socketEscucha, backLog) != 0) {
		perror("Error al poner a escuchar socket");
	} else {
		printf("Escuchando conexiones entrantes.\n");
	}
	fd_set read_fs; // descriptores q estan lisots para leer
	fd_set master; //descriptores q q estan actualemnte conectados
	size_t tamanio; // hace positivo a la variable
	int socketNuevaConexion;
	int nbytesRecibidos;
	int max;
	struct sockaddr_in cliente;

	FD_ZERO(&master);
	FD_ZERO(&read_fs);
	char *buffer = malloc(100 * sizeof(char));

	FD_SET(socketEscucha, &master);
	max = socketEscucha;

	while (1) {

		memcpy(&read_fs, &master, sizeof(master));
		int dev_select;
		if ((dev_select = select(max + 1, &read_fs, NULL, NULL, NULL)) == -1) {
			perror("select");

		}
		//printf("select = %d \n",dev_select);
		int i;
		//max : cantidad max de sockets
		for (i = 0; i <= max; i++) {
			if (FD_ISSET(i, &read_fs)) {
				//  printf("i = %d \n max = %d \n",i,max);
				if (i == socketEscucha) {
					// pasar a una funcion generica aceptar();
					tamanio = sizeof(struct sockaddr_in);
					if ((socketNuevaConexion = accept(socketEscucha,
							(struct sockaddr*) &cliente, &tamanio)) < 0) {
						perror("Error al aceptar conexion entrante");
					} else {
						if (socketNuevaConexion > max) {
							max = socketNuevaConexion;
						}
						FD_SET(socketNuevaConexion, &master);
						//printf("nueva conexion de %s desde socket %d \n",inet_ntoa(cliente.sin_addr), socketNuevaConexion);
					} //if del accept. Recibir hasta BUFF_SIZE datos y almacenarlos en 'buffer'.
				} else {

					//verifica si esta en el cojunto de listos para leer
					//pasarlo a una funcion generica
					if ((nbytesRecibidos = recv(i, buffer, BUFF_SIZE, 0)) > 0) {

						printf("RECIBIIIIIIIIIIIIIIIIII %s\n", buffer);
						int offset = 0, tmp_size = 0, code;
						tmp_size = sizeof(code);
						memcpy(&code, buffer, tmp_size);
						offset += tmp_size;

						t_bloque* bloque = malloc(sizeof(t_bloque));
						bloque->data = buffer + offset;
						bloque->size = strlen(bloque->data);

						switch (code) {
						case 100:
							aceptar_conexion_nodo(bloque, i);
							break;
						case 200:
							guardar_socket_marta(i);
							break;
						}
					} else {
						printf("no recibi una mierda !!! \n");
					}
				}
			}
		}
	}
}

int crear_db_directorios() {
	int ret;
	if ((ret = db_create(&dbHandler, NULL, 0)) != 0) {
		fprintf(stderr, "db_create: %s\n", db_strerror(ret));
		return EXIT_FAILURE;
	}
	if ((ret = dbHandler->open(dbHandler,
	NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
		dbHandler->err(dbHandler, ret, "%s", DATABASE);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int main(void) {
	levantar_configuracion();
	STATUS = false;
	ESPACIO_LIBRE_TOTAL = 0;
	NODOS_CONECTADOS = list_create();
	NODOS_ESPERANDO_CONEXION = list_create();
	crear_logger();
	log_info(LOGGER, "Puerto listen: %d\n", PUERTO_LISTEN);
	int res;
	res = crear_db_directorios();
	if (res != 0) {
		log_error(LOGGER,
				"La base de datos de los directorios no pudo ser creada.");
		return EXIT_FAILURE;
	}

// Crea el hilo que se encarga de las conexiones entrantes
	pthread_t thr_conexiones;
	int rcx;
	rcx = pthread_create(&thr_conexiones, NULL, (void *) conexiones_file_system,
	NULL);
	if (rcx != 0) {
		log_error(LOGGER,
				"El thread que maneja las conexiones no pudo ser creado.");
		return EXIT_FAILURE;
	}

// Crea el hilo que se encarga de la consola
//	pthread_t thr_consola;
//	int rc1;
//	rc1 = pthread_create(&thr_consola, NULL, (void *) consola_file_system,
//	NULL);
//	if (rc1 != 0) {
//		log_error(LOGGER, "El thread de la consola no pudo ser creado.");
//		return EXIT_FAILURE;
//	}

// Termina su ejecucion
	pthread_join(thr_conexiones, NULL);
//	pthread_join(thr_consola, NULL);
	log_destroy(LOGGER);
	return EXIT_SUCCESS;
}
