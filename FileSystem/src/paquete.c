#include "paquete.h"


char* paquete_serializar_mensaje(t_paquete* paquete, int* largo_paquete) {
	int offset = 0;
	int largo_data = paquete->largo_data;
	int largo_mensaje = sizeof(paquete->cod_op) + sizeof(largo_data) + largo_data;
	char* paquete_serializado = malloc(largo_mensaje + sizeof(largo_mensaje));

	paquete_serializar(paquete_serializado, &largo_mensaje, sizeof(largo_mensaje), &offset);

	paquete_serializar(paquete_serializado, &paquete->cod_op, sizeof(paquete->cod_op), &offset);

	paquete_serializar(paquete_serializado, &largo_data, sizeof(largo_data), &offset);

	paquete_serializar(paquete_serializado, paquete->data, largo_data, &offset);

	(*largo_paquete) = offset;

	return paquete_serializado;
}

t_paquete* paquete_deserializar_mensaje(char* mensaje) {
	int offset = 0;
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete_deserializar(&paquete->cod_op, mensaje, sizeof(paquete->cod_op), &offset);

	paquete_deserializar(&paquete->largo_data, mensaje, sizeof(paquete->largo_data), &offset);

	int largo_data = paquete->largo_data;
	paquete->data = malloc(largo_data);

	paquete_deserializar(paquete->data, mensaje, largo_data, &offset);

	return paquete;
}

void paquete_deserializar(void* to, void* from, int size, int* offset) {
	memcpy(to, from + (*offset), size);
	(*offset) += size;
}

void paquete_serializar(void* to, void* from, int size, int* offset) {
	memcpy(to + (*offset), from, size);
	(*offset) += size;
}
