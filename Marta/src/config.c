#include "config.h"

void leer_archivo_configuracion(char** ip_mdfs, uint16_t* puerto_mdfs, int* carga_map, int* carga_reduce) {
	t_config* config = NULL;

	config = config_create(ARCHIVO_CONFIG);

	if (config == NULL) {
		log_error_consola("No se pudo abrir el archivo de configuracion");
		exit(1);
	}

	log_debug_consola("Leyendo archivo de configuracion");

	if (config_has_property(config, "IP_MDFS")) {
		(*ip_mdfs) = config_get_string_value(config, "IP_MDFS");
	} else {
		log_error_consola("El archivo de configuracion debe tener un IP_MDFS");
		exit(1);
	}
	if (config_has_property(config, "PUERTO_MDFS")) {
		(*puerto_mdfs) = config_get_int_value(config, "PUERTO_MDFS");
	} else {
		log_error_consola("El archivo de configuracion debe tener un PUERTO_MDFS");
		exit(1);
	}
	if (config_has_property(config, "CARGA_MAP")) {
		(*carga_map) = config_get_int_value(config, "CARGA_MAP");
	} else {
		log_error_consola("El archivo de configuracion debe tener un CARGA_MAP");
		exit(1);
	}
	if (config_has_property(config, "CARGA_REDUCE")) {
		(*carga_reduce) = config_get_int_value(config, "CARGA_REDUCE");
	} else {
		log_error_consola("El archivo de configuracion debe tener un CARGA_REDUCE");
		exit(1);
	}

	log_info_consola("Archivo de configuracion cargado correctamente");
}
