/*
 * FileSystem.c
 *
 *  Created on: 1/5/2015
 *      Author: tomasyagas
 */

#include "FileSystem.h"

uint16_t PUERTO_LISTEN;
int SOCKET_MARTA;
unsigned int CANTIDAD_NODOS_MINIMA;
t_list* NODOS_CONECTADOS;
t_list* NODOS_ESPERANDO_CONEXION;
t_log* LOGGER;
bool STATUS;
unsigned int ESPACIO_LIBRE_TOTAL;
//DB *dbHandler;

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
	if (list_size(NODOS_ESPERANDO_CONEXION) > 0) {
		t_nodo *nodo = (t_nodo*) list_remove(NODOS_ESPERANDO_CONEXION, 0);
		nodo->cantidad_bloques_libres = 50;
		nodo->conectado_andando = true;

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
		log_error(LOGGER, "No se pudo agregar el nodo a la lista de nodos.");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void agregar_nodo_viejo(int id, int socket) {
	void* buscar_nodo(t_nodo* iterador) {
		return iterador->id == id;
	}
	t_nodo* nodo = (t_nodo*) list_find(NODOS_CONECTADOS,
			(bool (*)(void*)) buscar_nodo);
	nodo->socket = socket;
	nodo->conectado_andando = true;
	ESPACIO_LIBRE_TOTAL = ESPACIO_LIBRE_TOTAL
			+ ((nodo->cantidad_bloques_libres) * 20480);
}

void guardar_socket_marta(int socket) {
	SOCKET_MARTA = socket;
}

void *atenderConexiones(void *parametro) {
	t_msg *msg;
	int sock_conexion = *((int *) parametro);
	int nodo_nuevo, nodo_id;

	while (1) {
		if ((msg = recibir_mensaje(sock_conexion)) != NULL) {

			switch (msg->header.id) {
			case INFO_NODO:
				nodo_nuevo = msg->argv[0];
				nodo_id = msg->argv[1];
				log_info(LOGGER, "NODO CONECTADO: %d\n", nodo_id);
				if (nodo_nuevo) {
					agregar_nodo_nuevo(nodo_id, sock_conexion);
				} else {
					agregar_nodo_viejo(nodo_id, sock_conexion);
				}
				break;
//			default:
//				pthread_mutex_lock(&LogMutex);
//				log_warning(Logger, "Recepción de solicitud inválida.");
//				pthread_mutex_unlock(&LogMutex);
			}
		} else {
//			log_warning(Logger, "Desconexión de un proceso.");
			break;
		}
		destroy_message(msg);
	}
	return NULL;
}

void conexiones_file_system() {
	pthread_t thread;
	int listener, nuevaConexion;

	listener = server_socket(PUERTO_LISTEN);
	printf("Esperando conexiones entrantes\n");
	while (true) {
		nuevaConexion = accept_connection(listener);
//			pthread_mutex_lock(&LogMutex);
//			log_trace(Logger, "Nueva conexión.");
//			pthread_mutex_unlock(&LogMutex);
		pthread_create(&thread, NULL, atenderConexiones, &nuevaConexion);
	}
}

/*
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
//	DBT key, data;
//	memset(&key, 0, sizeof(key));
//	memset(&data, 0, sizeof(data));
//	key.data = "fruit";
//	key.size = sizeof("fruit");
//	printf("Sizeof(fruit) == %d\n", key.size);
//	data.data = "apple";
//	data.size = sizeof("apple");
//	printf("Sizeof(apple) == %d\n", data.size);
//
//	if ((ret = dbHandler->put(dbHandler, NULL, &key, &data, 0)) == 0) {
//		printf("db: %s: key stored.\n", (char *) key.data);
//		return EXIT_SUCCESS;
//	} else {
//		dbHandler->err(dbHandler, ret, "DB->put");
//		return EXIT_FAILURE;
//	}
	return EXIT_SUCCESS;
} */

int main(void) {
	levantar_configuracion();
	STATUS = false;
	ESPACIO_LIBRE_TOTAL = 0;
	NODOS_CONECTADOS = list_create();
	NODOS_ESPERANDO_CONEXION = list_create();
	crear_logger();
	log_info(LOGGER, "Puerto listen: %d\n", PUERTO_LISTEN);

//	int res;
//	res = crear_db_directorios();
//	if (res != 0) {
//		log_error(LOGGER,
//				"La base de datos de los directorios no pudo ser creada.");
//		return EXIT_FAILURE;
//	}

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
