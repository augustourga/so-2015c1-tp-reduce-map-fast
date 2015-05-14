/*
 * FileSystem.c
 *
 *  Created on: 1/5/2015
 *      Author: utnso
 */

#include "FileSystem.h"

int PUERTO_LISTEN;
char** LISTA_NODOS;
t_log* LOGGER;
bool STATUS;

void levantar_configuracion() {
	t_config* config;
	config = config_create("config_file_system.cfg");
	if (config_has_property(config, "PUERTO_LISTEN")) {
		PUERTO_LISTEN = config_get_int_value(config, "PUERTO_LISTEN");
	}
	if (config_has_property(config, "LISTA_NODOS")) {
		LISTA_NODOS = config_get_array_value(config, "LISTA_NODOS");
	}
	config_destroy(config);
}

void crear_logger() {
	LOGGER = log_create("log", "FileSystem", true, LOG_LEVEL_INFO);
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

void manejar_conexiones_nuevas(int socket) {
	int length, bytes_recv;
	char *buffer;
	length = 1024;
	bytes_recv = recv(socket, buffer, length, 0);

	// guardar el new_fd dependiendo de lo que responda
	// puede ser nodo nuevo o viejo
	// o puede ser Marta

}

void conexiones_file_system() {
// Abre su puerto de escucha
	int backLog = 20; // Número de conexiones permitidas en la cola de entrada (hasta que se aceptan)
	int socketEscucha;
	socketEscucha = crear_socket_listen(PUERTO_LISTEN);
	printf("Socket: %d\n", socketEscucha);
	listen(socketEscucha, backLog);

	int sin_size;
	struct sockaddr_in their_addr; // Información sobre la dirección remota
	sin_size = sizeof(struct sockaddr_in);
// Acepta nuevas conexiones
	while (1) {
		int new_fd, len, length, rcx;
		new_fd = accept(socketEscucha, (struct sockaddr *) &their_addr,
				&sin_size);
		pthread_t thr_aceptar_conexiones;
		rcx = pthread_create(&thr_aceptar_conexiones, NULL,
				(void *) manejar_conexiones_nuevas, &new_fd);
		if (rcx != 0) {
			log_error(LOGGER,
					"El thread que acepta las conexiones entrantes no pudo ser creado.");
		}
	}
}

int main(void) {
	levantar_configuracion();
	STATUS = false;
	crear_logger();
	log_info(LOGGER, "Puerto listen: %d\n", PUERTO_LISTEN);

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
	pthread_t thr_consola;
	int rc1;
	rc1 = pthread_create(&thr_consola, NULL, (void *) consola_file_system,
	NULL);
	if (rc1 != 0) {
		log_error(LOGGER, "El thread de la consola no pudo ser creado.");
		return EXIT_FAILURE;
	}

// Termina su ejecucion
	pthread_join(thr_conexiones, NULL);
	pthread_join(thr_consola, NULL);
	log_destroy(LOGGER);
	return EXIT_SUCCESS;
}
