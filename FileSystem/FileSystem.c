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

int main(void) {
	levantar_configuracion();
	STATUS = false;
	crear_logger();
	log_info(LOGGER,"Puerto listen: %d\n",PUERTO_LISTEN);

	int socket;
	socket = obtener_socket();
	printf("Socket: %d\n",socket);
	log_destroy(LOGGER);
	return EXIT_SUCCESS;
}
