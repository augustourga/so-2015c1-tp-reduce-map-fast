#include "config.h"

void leer_archivo_configuracion(char** ip_mdfs, uint16_t* puerto_mdfs, int* operaciones_por_nodo) {
	t_config* config = NULL;

	config = config_create(ARCHIVO_CONFIG);

	if (config == NULL) {
		log_error_consola("No se pudo abrir el archivo de configuración");
		exit(1);
	}

	log_debug_consola("Leyendo archivo de configuración");

	if (config_has_property(config, "IP_MDFS")) {
		(*ip_mdfs) = config_get_string_value(config, "IP_MDFS");
	} else {
		log_error_consola("El archivo de configuración debe tener un IP_MDFS");
		exit(1);
	}
	if (config_has_property(config, "PUERTO_MDFS")) {
		(*puerto_mdfs) = config_get_int_value(config, "PUERTO_MDFS");
	} else {
		log_error_consola("El archivo de configuración debe tener un PUERTO_MDFS");
		exit(1);
	}
	if (config_has_property(config, "OPERACIONES_NODO")) {
		(*puerto_mdfs) = config_get_string_value(config, "OPERACIONES_NODO");
	} else {
		log_error_consola("El archivo de configuración debe tener un OPERACIONES_NODO");
		exit(1);
	}

	log_info_consola("Archivo de configuración cargado correctamente");
}