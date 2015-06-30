/*
 * Marta.c
 *
 *  Created on: 1/5/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <time.h>
#include <memory.h>

#include "marta.h"
#define MY_NULL '\0'
#define MYLEN 5

char* ip_mdfs;

t_list* lista_nodos;
t_list* lista_tareas;

int main(int argc, char* argv[]) {

	uint16_t puerto_mdfs;
	int operaciones_por_nodo;

	lista_nodos = list_create();
	lista_tareas = list_create();

	log_crear(argv[1], RUTA_LOG, "MaRTA");

	leer_archivo_configuracion(&ip_mdfs, &puerto_mdfs, &operaciones_por_nodo);

	conectarse_a_mdfs(ip_mdfs, puerto_mdfs);

	iniciar_server((uint16_t) PUERTO_LISTEN);

	return 0;
}

int conectarse_a_mdfs(char* ip_mdfs, uint16_t puerto_mdfs) {
	int socket_mdfs;

	if ((socket_mdfs = client_socket(ip_mdfs, puerto_mdfs)) < 0) {
		log_error_consola("Error al conectarse a MDFS");
		exit(1);
	}

	t_msg* mensaje = id_message(CONEXION_MARTA);
	enviar_mensaje(socket_mdfs, mensaje);

	destroy_message(mensaje);

	return socket_mdfs;
}

char* getRandName(char* fileName, char* indicador) {
	char* guion;
	char s[MYLEN], *ptr;
	int i, num, start, range;
	long seconds;
	start = (int) ('A');
	range = (int) ('Z') - (int) ('A');
	time(&seconds);
	srand((unsigned int) seconds);
	for (ptr = s, i = 1; i < MYLEN; ++ptr, ++i) {
		num = rand() % range;
		*ptr = (char) (num + start);
	}
	*ptr = MY_NULL;
	puts(s);
	guion = "_";
	char* strFinal = malloc(1 + strlen(fileName) + strlen(indicador));
	strcpy(strFinal, fileName);
	printf("%s", strFinal);
	strcat(strFinal, guion);
	puts(strFinal);
	strcat(strFinal, indicador);
	strcat(strFinal, guion);
	strcat(strFinal, s);
	return strFinal;
}
