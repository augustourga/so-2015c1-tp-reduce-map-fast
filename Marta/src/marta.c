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

int main(int argc, char* argv[]) {

	uint16_t puerto_mdfs;
	char* ip_mdfs;

	lista_nodos = list_create();
	lista_jobs = list_create();

	log_crear(argv[1], RUTA_LOG, "MaRTA");

	leer_archivo_configuracion(&ip_mdfs, &puerto_mdfs);

	conectarse_a_mdfs(ip_mdfs, puerto_mdfs);

	iniciar_server((uint16_t) PUERTO_LISTEN);

	return 0;
}

void generar_maps(t_job* job, char* ruta_mdfs) {

	char* info_archivo = get_info_archivo(ruta_mdfs);
	int bloque_archivo = 0;

	char** bloques_archivo = string_split(info_archivo, "|");

	void _crear_map(char* bloque_archivo_info) {
		char** datos = string_split(bloque_archivo_info, ";");

		t_map* map = map_crear();

		t_archivo archivo;
		archivo.nombre = string_duplicate(ruta_mdfs);
		archivo.bloque = bloque_archivo;

		int i;
		int copia;
		for (i = 0, copia = 0; datos[i] != NULL && copia < 3; i += 4, copia++) {
			archivo.copias[copia].nombre = string_duplicate(datos[i]);
			archivo.copias[copia].ip = string_duplicate(datos[i + 1]);
			archivo.copias[copia].puerto = atoi(datos[i + 2]);
			archivo.copias[copia].numero_bloque = atoi(datos[i + 3]);
			agregar_nodo_si_no_existe(archivo.copias[copia]);
		}

		map->archivo = archivo;

		list_add(job->maps, map);

		bloque_archivo++;
	}

	string_iterate_lines(bloques_archivo, (void*) _crear_map);

}

void _planifica_map(t_map* map) {
	if (map->estado == PENDIENTE || map->estado == FIN_ERROR) {
		t_temp arch_tmp;
		arch_tmp.nombre = getRandName(map->archivo.nombre, "????"); //TODO: No se qué va acá
		arch_tmp.nodo = get_nodo_menos_cargado(map->archivo.copias);
		map->arch_tmp = arch_tmp;
	}
}

void planifica_maps(t_job* job) {
	list_iterate(job->maps, (void*) _planifica_map);
}

void planificar_reduces_con_combiner(t_job* job) {
	bool _ordena_por_nombre(t_map* map1, t_map* map2) {
		return map1->arch_tmp.nombre <= map2->arch_tmp.nombre;
	}

	list_sort(job->maps, (void*) _ordena_por_nombre);
	t_map* primer_map = list_get(job->maps, 0);
	char* nombre_actual = primer_map->arch_tmp.nodo.nombre;
	t_temp* temp_actual = malloc(sizeof(t_temp));
	temp_actual->nodo = primer_map->arch_tmp.nodo;
	t_reduce* reduce_actual = reduce_crear();
	reduce_actual->arch_tmp.nodo = primer_map->arch_tmp.nodo;
	reduce_actual->arch_tmp.nombre = getRandName("Sarasa1", "sarasa2"); //TODO: Generar nombre de archivo

	void _genera_reduces(t_map* map) {
		if (!strcmp(nombre_actual, map->arch_tmp.nodo.nombre)) {
			string_append(&temp_actual->nombre, map->arch_tmp.nombre);
			string_append(&temp_actual->nombre, "|");
		} else {
			list_add(reduce_actual->temporales, temp_actual);
			list_add(job->reduces, reduce_actual);
			t_reduce* reduce_actual = reduce_crear();
			reduce_actual->arch_tmp.nodo = map->arch_tmp.nodo;
			reduce_actual->arch_tmp.nombre = getRandName("Sarasa1", "sarasa2"); //TODO: Generar nombre de archivo
			temp_actual = malloc(sizeof(t_temp));
			nombre_actual = string_duplicate(map->arch_tmp.nodo.nombre);
			temp_actual->nodo = map->arch_tmp.nodo;
			temp_actual->nombre = string_duplicate(map->arch_tmp.nombre);
			string_append(&temp_actual->nombre, "|");
		}
	}

	list_iterate(job->maps, (void*) _genera_reduces);

	t_reduce* primer_reduce = list_get(job->reduces, 0);

	t_reduce* reduce_final = reduce_crear();
	reduce_final->arch_tmp.nodo = primer_reduce->arch_tmp.nodo; //TODO: ver si se puede elegir otro con algún criterio mejor
	reduce_final->arch_tmp.nombre = getRandName("Saras", "jojo"); //TODO: Generar nombre

	void _temporales_reduce_final(t_reduce* reduce) {
		list_add(reduce_final->temporales, &reduce->arch_tmp);
	}

	list_iterate(job->reduces, (void*) _temporales_reduce_final);

	job->reduce_final = reduce_final;

}

void planificar_reduces_sin_combiner(t_job* job) {

	t_reduce* reduce = reduce_crear();

	t_dictionary* dictionary = dictionary_create();

	void _contabilizar_nodos(t_map* map) {
		if (dictionary_has_key(dictionary, map->arch_tmp.nodo.nombre)) {
			dictionary_put(dictionary, map->arch_tmp.nodo.nombre, dictionary_get(dictionary, map->arch_tmp.nodo.nombre) + 1);
		} else {
			dictionary_put(dictionary, map->arch_tmp.nodo.nombre, (void*) 1);
		}
	}

	list_iterate(job->maps, (void*) _contabilizar_nodos);

	int max = 1;
	char* nombre_nodo_con_mas_archivos;
	void _nodo_con_mas_archivos(char* nombre, int value) {
		if (value >= max) {
			nombre_nodo_con_mas_archivos = string_duplicate(nombre);
			max = value;
		}
	}

	dictionary_iterator(dictionary, (void*) _nodo_con_mas_archivos);

	pthread_mutex_lock(&mutex_nodos);

	bool _nodo_por_nombre(t_nodo_global* nodo_global_actual) {
		return !strcmp(nodo_global_actual->nodo.nombre, nombre_nodo_con_mas_archivos);
	}

	t_nodo_global* nodo_global = list_find(lista_nodos, (void*) _nodo_por_nombre);

	reduce->arch_tmp.nodo = nodo_global->nodo;
	reduce->arch_tmp.nombre = getRandName("pepe", "????"); //TODO: No se qué va acá, generar un nombre para las salidas de los reduces

	bool _ordena_por_nombre(t_map* map1, t_map* map2) {
		return map1->arch_tmp.nombre <= map2->arch_tmp.nombre;
	}

	list_sort(job->maps, (void*) _ordena_por_nombre);
	t_map* primer_map = list_get(job->maps, 0);
	char* nombre_actual = primer_map->arch_tmp.nodo.nombre;
	t_temp* temp_actual = malloc(sizeof(t_temp));
	temp_actual->nodo = primer_map->arch_tmp.nodo;

	void _genera_temporales(t_map* map) {
		if (!strcmp(nombre_actual, map->arch_tmp.nodo.nombre)) {
			string_append(&temp_actual->nombre, map->arch_tmp.nombre);
			string_append(&temp_actual->nombre, "|");
		} else {
			list_add(reduce->temporales, temp_actual);
			temp_actual = malloc(sizeof(t_temp));
			nombre_actual = string_duplicate(map->arch_tmp.nodo.nombre);
			temp_actual->nodo = map->arch_tmp.nodo;
			temp_actual->nombre = string_duplicate(map->arch_tmp.nombre);
			string_append(&temp_actual->nombre, "|");
		}
	}

	list_iterate(job->maps, (void*) _genera_temporales);

	job->reduce_final = reduce;

	pthread_mutex_unlock(&mutex_nodos);

}

void planifica_reduces(t_job* job) {

	if (job->combiner) {
		planificar_reduces_con_combiner(job);
	} else {
		planificar_reduces_sin_combiner(job);
	}

}

void procesa_job(void* argumentos) {

	t_job* job = job_crear();

	struct arg_job* args = argumentos;

	job->socket = args->socket;

	char** datos = string_split(args->mensaje->stream, "|");

	job->archivo_final = string_duplicate(datos[0]);

	int i;
	for (i = 1; datos[i + 1] != NULL; i++) {
		generar_maps(job, datos[i]);
	}
	job->combiner = datos[i];

	lista_jobs_add(job);

	planifica_maps(job);

	ejecuta_maps(job);

	sem_wait(&job->sem_maps_fin);

	planifica_reduces(job);

	if (job->combiner) {
		ejecuta_reduces_parciales(job);

		sem_wait(&job->sem_reduces_fin);
	}

	ejecuta_reduce_final(job);

	sem_wait(&job->sem_reduce_final_fin);
}

t_nodo get_nodo_menos_cargado(t_nodo nodos[3]) {

	pthread_mutex_lock(&mutex_nodos);

	t_nodo_global* nodo_global[3] = { 0 };
	t_nodo ret;

	int i;
	for (i = 0; i < 3; i++) {

		bool _nodo_valido(t_nodo_global* nodo) {
			return !strcmp(nodo->nodo.nombre, nodos[i].nombre);
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
		return (!strcmp(nodo_global->nodo.nombre, nodo_nuevo.nombre));
	}

	nodo_existente = list_find(lista_nodos, (void*) _nodo_por_nombre);

	if (nodo_existente == NULL) {
		nodo_existente = malloc(sizeof(t_nodo_global));
		nodo_existente->carga_trabajo = 0;
		nodo_existente->nodo = nodo_nuevo;

		list_add(lista_nodos, nodo_existente);
	}
	pthread_mutex_unlock(&mutex_nodos);
}

t_job* job_por_socket(int socket) {
	t_job* job_actual = NULL;

	bool _job_por_socket(t_job* job) {
		return job->socket == socket;
	}

	job_actual = list_find(lista_jobs, (void*) _job_por_socket);

	return job_actual;
}

t_map* map_por_id(int id, t_job* job) {
	t_map* map_actual = NULL;

	bool _map_por_id(t_map* map) {
		return map->id == id;
	}

	map_actual = list_find(job->maps, (void*) _map_por_id);

	return map_actual;
}

t_reduce* reduce_por_id(int id, t_job* job) {
	t_reduce* reduce_actual = NULL;

	bool _map_por_id(t_reduce* reduce) {
		return reduce->id == id;
	}

	reduce_actual = list_find(job->reduces, (void*) _map_por_id);

	return reduce_actual;
}

void actualiza_job_map_ok(int id, int socket) {
	pthread_mutex_lock(&mutex_jobs);

	t_job* job_actual = job_por_socket(socket);

	if (!job_actual) {
		log_error_consola("El job no existe");
		exit(1);
	}

	t_map* map_actual = map_por_id(id, job_actual);

	if (!map_actual) {
		log_error_consola("El map no existe");
		exit(1);
	}

	map_actual->estado = FIN_OK;

	bool _finalizo_ok(t_map* map) {
		return map->estado == FIN_OK;
	}

	if (list_all_satisfy(job_actual->maps, (void*) _finalizo_ok)) {
		sem_post(&job_actual->sem_maps_fin);
	}

	pthread_mutex_unlock(&mutex_jobs);
}

void elimina_nodo_desconectado(char* nombre_nodo) {
	pthread_mutex_lock(&mutex_nodos);

	bool _nodo_por_nombre(t_nodo_global* nodo_global_actual) {
		return !strcmp(nodo_global_actual->nodo.nombre, nombre_nodo);
	}

	list_remove_and_destroy_by_condition(lista_nodos, (void*) _nodo_por_nombre, (void*) destroy_nodo);

	pthread_mutex_unlock(&mutex_nodos);
}

void actualiza_job_map_error(int id, int socket) {
	pthread_mutex_lock(&mutex_jobs);

	t_job* job_actual = job_por_socket(socket);

	if (!job_actual) {
		log_error_consola("El job no existe");
		exit(1);
	}

	t_map* map_actual = map_por_id(id, job_actual);

	if (!map_actual) {
		log_error_consola("El map no existe");
		exit(1);
	}

	map_actual->estado = FIN_ERROR;

	elimina_nodo_desconectado(map_actual->arch_tmp.nodo.nombre);

	_planifica_map(map_actual);

	ejecuta_maps(job_actual);

	pthread_mutex_unlock(&mutex_jobs);
}

void actualiza_job_reduce_ok(int id, int socket) {
	pthread_mutex_lock(&mutex_jobs);

	t_job* job_actual = job_por_socket(socket);

	if (!job_actual) {
		log_error_consola("El job no existe");
		exit(1);
	}

	t_reduce* reduce_actual = reduce_por_id(id, job_actual);

	if (!reduce_actual) {
		log_error_consola("El map no existe");
		exit(1);
	}

	if (id == job_actual->reduce_final->id) {
		copiar_archivo_final(job_actual);
		sem_post(&job_actual->sem_reduce_final_fin);
	} else {
		reduce_actual->estado = FIN_OK;

		bool _finalizo_ok(t_reduce* reduce) {
			return reduce->estado == FIN_OK;
		}

		if (list_all_satisfy(job_actual->reduces, (void*) _finalizo_ok)) {
			sem_post(&job_actual->sem_reduces_fin);
		}
	}
	pthread_mutex_unlock(&mutex_jobs);
}

void actualizar_job_reduce_error(int id, int socket, char* nombre_nodo) {
	//TODO: Ejecutar tareas de limpieza por cancelación de job
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

void destroy_nodo(t_nodo_global* nodo) {
	free(nodo->nodo.ip);
	free(nodo->nodo.nombre);
	free(nodo);
}
