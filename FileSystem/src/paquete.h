/*
 * paquete.h
 *
 *  Created on: 12/6/2015
 *      Author: utnso
 */

#ifndef PAQUETE_H_
#define PAQUETE_H_

#include <stdlib.h>
#include <string.h>

void paquete_serializar(void* to, void* from, int size, int* offset);
void paquete_deserializar(void* to, void* from, int size, int* offset);

#endif /* PAQUETE_H_ */
