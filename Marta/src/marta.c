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

t_list* lista_nodos;
t_list* lista_jobs;

pthread_mutex_t mutex_jobs = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_nodos = PTHREAD_MUTEX_INITIALIZER;

int socket_mdfs;

int main(int argc, char* argv[]) {

	uint16_t puerto_mdfs;
	char* ip_mdfs;
	int operaciones_por_nodo;

	lista_nodos = list_create();
	lista_jobs = list_create();

	log_crear(argv[1], RUTA_LOG, "MaRTA");

	leer_archivo_configuracion(&ip_mdfs, &puerto_mdfs, &operaciones_por_nodo);

	conectarse_a_mdfs(ip_mdfs, puerto_mdfs);

	iniciar_server((uint16_t) PUERTO_LISTEN);

	return 0;
}

void planificar_maps(t_job** job) {

	void _planifica_maps(t_map* map) {
		t_temp arch_tmp;
		arch_tmp.nombre = getRandName(map->archivo.nombre, "????"); //TODO: No se qué va acá
		arch_tmp.nodo = get_nodo_menos_cargado(map->archivo.copias);
		map->arch_tmp = arch_tmp;
	}

	list_iterate((*job)->maps, (void*) _planifica_maps);

}

void procesar_job(void* argumentos) {

	t_job* job = job_crear();

	struct arg_job* args = argumentos;

	job->socket = args->socket;

	char** datos = string_split(args->mensaje->stream, "|");

	job->archivo_final = string_duplicate(datos[0]);

	int i;
	for (i = 1; datos[i + 1] != NULL; i++) {
		generar_maps(&job, datos[i]);
	}
	job->combiner = datos[i];

	lista_jobs_add(job);

	planificar_maps(&job);

	ejecutar_maps(job);

	//planificar_reduces(&job); //TODO: Planificar reduces

	//copiar_archivo_final(); //TODO: copiar archivo final

}

t_nodo get_nodo_menos_cargado(t_nodo nodos[3]) {

	pthread_mutex_lock(&mutex_nodos);

	t_nodo_global* nodo_global[3] = { 0 };
	t_nodo ret;

	int i;
	for (i = 0; i < 3; i++) {

		bool _nodo_valido(t_nodo_global* nodo) {
			return !strcmp(nodo->nombre, nodos[i].nombre);
		}

		if (nodos[i].nombre != NULL) {
			nodo_global[i] = list_find(lista_nodos, (void*) _nodo_valido);
		}
	}

	//TODO: Capaz cambiarlo porque esta BIEN choto
	if (nodo_global[0]->carga_trabajo <= nodo_global[1]->carga_trabajo && nodo_global[0]->carga_trabajo <= nodo_global[2]->carga_trabajo) {
		ret = nodos[0];
	}
	if (nodo_global[1]->carga_trabajo <= nodo_global[0]->carga_trabajo && nodo_global[1]->carga_trabajo <= nodo_global[2]->carga_trabajo) {
		ret = nodos[1];
	}
	if (nodo_global[2]->carga_trabajo <= nodo_global[0]->carga_trabajo && nodo_global[2]->carga_trabajo <= nodo_global[1]->carga_trabajo) {
		ret = nodos[2];
	}

	pthread_mutex_unlock(&mutex_nodos);
	return ret;
}

char* getRandName(char* fileName, char* indicador) {
	char* guion;
	char s[5], *ptr;
	int i, num, start, range;
	long seconds;
	start = (int) ('A');
	range = (int) ('Z') - (int) ('A');
	time(&seconds);
	srand((unsigned int) seconds);
	for (ptr = s, i = 1; i < 5; ++ptr, ++i) {
		num = rand() % range;
		*ptr = (char) (num + start);
	}
	*ptr = '\0';
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

void agregar_nodo_si_no_existe(t_nodo nodo_nuevo) {
	pthread_mutex_lock(&mutex_nodos);

	t_nodo_global* nodo_existente = NULL;

	bool _nodo_por_nombre(t_nodo_global* nodo_global) {
		return (!strcmp(nodo_global->nombre, nodo_nuevo.nombre));
	}

	nodo_existente = list_find(lista_nodos, (void*) _nodo_por_nombre);

	if (nodo_existente == NULL) {
		nodo_existente = malloc(sizeof(t_nodo_global));
		nodo_existente->carga_trabajo = 0;
		nodo_existente->ip = string_duplicate(nodo_nuevo.ip);
		nodo_existente->nombre = string_duplicate(nodo_nuevo.nombre);
		nodo_existente->puerto = nodo_nuevo.puerto;

		list_add(lista_nodos, nodo_existente);
	}
	pthread_mutex_unlock(&mutex_nodos);
}

void lista_jobs_add(t_job* job) {
	pthread_mutex_lock(&mutex_jobs);
	list_add(lista_jobs, job);
	pthread_mutex_unlock(&mutex_jobs);
}

void lista_nodos_add(t_nodo_global* nodo) {
	pthread_mutex_lock(&mutex_nodos);
	list_add(lista_nodos, nodo);
	pthread_mutex_unlock(&mutex_nodos);
}
