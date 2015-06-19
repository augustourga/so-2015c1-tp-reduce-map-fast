#include "config.h"


void leer_archivo_configuracion(char** puerto_listen, char** cantidad_nodos) {
	t_config* config = NULL;

	config = config_create(ARCHIVO_CONFIG);

	if (config == NULL) {
		log_error_consola("No se pudo abrir el archivo de configuración");
		exit(1);
	}

	log_debug_consola("Leyendo archivo de configuración");

	if (config_has_property(config, "PUERTO_LISTEN")) {
		(*puerto_listen) = config_get_string_value(config, "PUERTO_LISTEN");
	} else {
		log_error_consola("El archivo de configuración debe tener un PUERTO_LISTEN");
		exit(1);
	}
	if (config_has_property(config, "CANTIDAD_NODOS")) {
		(*cantidad_nodos) = config_get_string_value(config, "CANTIDAD_NODOS");
	} else {
		log_error_consola("El archivo de configuración debe tener una CANTIDAD_NODOS");
		exit(1);
	}
	log_info_consola("Archivo de configuración cargado correctamente");
}
