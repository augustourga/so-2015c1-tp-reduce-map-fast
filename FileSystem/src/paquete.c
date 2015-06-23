#include "paquete.h"

void paquete_deserializar(void* to, void* from, int size, int* offset) {
	memcpy(to, from + (*offset), size);
	(*offset) += size;
}

void paquete_serializar(void* to, void* from, int size, int* offset) {
	memcpy(to + (*offset), from, size);
	(*offset) += size;
}
