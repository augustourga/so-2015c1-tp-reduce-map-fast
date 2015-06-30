#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <commons/config.h>
#include <stddef.h>
#define ARCHIVO_CONFIG "config/marta.conf"
#include <stdint.h>

void leer_archivo_configuracion(char** ip_mdfs, uint16_t* puerto_mdfs, int* operaciones_por_nodo);

#endif /* SRC_CONFIG_H_ */
