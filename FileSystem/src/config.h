
#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <commons/config.h>
#include <stddef.h>
#include <utiles/log.h>
#define ARCHIVO_CONFIG "./files/config_file_system.cfg"

void leer_archivo_configuracion(char** puerto_listen, char** cantidad_nodos);

#endif /* SRC_CONFIG_H_ */
