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
int carga_map;
int carga_reduce;

pthread_mutex_t mutex_jobs = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_nodos = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char* argv[]) {

	uint16_t puerto_mdfs;
	char* ip_mdfs;

	lista_nodos = list_create();
	lista_jobs = list_create();

	log_crear(argv[1], RUTA_LOG, "MaRTA");

	leer_archivo_configuracion(&ip_mdfs, &puerto_mdfs, &carga_map, &carga_reduce);

	conectarse_a_mdfs(ip_mdfs, puerto_mdfs);

	iniciar_server((uint16_t) PUERTO_LISTEN);

	return 0;
}

void finalizar_job_a_si_mismo(t_job* job) {

	pthread_mutex_lock(&mutex_jobs);

	bool _job(t_job* jobLista) {
			return jobLista == job;
		}
	list_remove_by_condition(lista_jobs, (void*)_job);
	pthread_mutex_unlock(&mutex_jobs);
	pthread_exit(NULL);
}

void generar_maps(t_job* job, char* ruta_mdfs) {
	log_debug_interno("Armando Maps para el archivo %s", &ruta_mdfs);
	char* info_archivo = get_info_archivo(ruta_mdfs);

	if (info_archivo == NULL) {
		log_info_consola("Error al obtener informacion del FS. Terminando ejecucion Job: %d", job->socket);
		finalizar_job_a_si_mismo(job);
	}

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



void planifica_maps(t_job* job) {
	log_debug_consola("Planificando maps Job: %d", job->socket);


	void _planifica_map(t_map* map) {
		if (map->estado == PENDIENTE || map->estado == FIN_ERROR) {
			t_temp arch_tmp;
			arch_tmp.nombre = getRandName(map->archivo.nombre,
					string_itoa(map->archivo.bloque));
			arch_tmp.nodo = get_nodo_menos_cargado(map->archivo.copias);

			if(arch_tmp.nodo.nombre == NULL){
				log_error_consola("Error encontrando Nodo Disponible. Job %d Cancelado.", job->socket);
				finalizar_job_a_si_mismo(job);
			}

			map->arch_tmp = arch_tmp;
		}
	}

	list_iterate(job->maps, (void*) _planifica_map);
	log_debug_consola("FIN planificacion maps Job: %d", job->socket);
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
			dictionary_put(dictionary, map->arch_tmp.nodo.nombre,
					dictionary_get(dictionary, map->arch_tmp.nodo.nombre) + 1);
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
		return !strcmp(nodo_global_actual->nodo.nombre,
				nombre_nodo_con_mas_archivos);
	}

	t_nodo_global* nodo_global = list_find(lista_nodos,
			(void*) _nodo_por_nombre);

	reduce->arch_tmp.nodo = nodo_global->nodo;
	reduce->arch_tmp.nombre = getRandName(string_itoa(job->socket), "RD_FINAL"); //TODO: No se qué va acá, generar un nombre para las salidas de los reduces

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
			string_append(&temp_actual->nombre, ";");
		} else {
			list_add(reduce->temporales, temp_actual);
			temp_actual = malloc(sizeof(t_temp));
			nombre_actual = string_duplicate(map->arch_tmp.nodo.nombre);
			temp_actual->nodo = map->arch_tmp.nodo;
			temp_actual->nombre = string_duplicate(map->arch_tmp.nombre);
			string_append(&temp_actual->nombre, ";");
		}
	}

	list_iterate(job->maps, (void*) _genera_temporales);

	job->reduce_final = reduce;
	nodo_global->carga_trabajo += carga_reduce;

	pthread_mutex_unlock(&mutex_nodos);

}

void planifica_reduces(t_job* job) {

	if (job->combiner) {
		log_debug_consola("planificando reduces CON combiner. job: %d", job->socket);
		planificar_reduces_con_combiner(job);
	} else {
		log_debug_consola("planificando reduces SIN combiner. job:  %d", job->socket);
		planificar_reduces_sin_combiner(job);
	}
	log_debug_consola("reduces planificados. job: %d", job->socket);
}

void procesa_job(void* argumentos) {

	t_job* job = job_crear();

	struct arg_job* args = argumentos;

	job->socket = args->socket;

	char** datos = string_split(args->stream, "|");

	job->archivo_final = string_duplicate(datos[0]);
	job->combiner = args->combiner; //TODO: Modificar en el Job
	free(args);

	int i;
	for (i = 1; datos[i] != NULL; i++) {
		generar_maps(job, datos[i]);
	}

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

	t_nodo ret;

	t_list* lista_nodos_globales_bloque = list_create();

	int i;
	for (i = 0; i < 3; i++) {
		bool _nodo_valido(t_nodo_global* nodo) {
			return !strcmp(nodo->nodo.nombre, nodos[i].nombre);
		}

		if (nodos[i].nombre != NULL) {
			t_nodo_global* nodo_global_activo = list_find(lista_nodos,
					(void*) _nodo_valido);
			if (nodo_global_activo != NULL) {
				list_add(lista_nodos_globales_bloque, nodo_global_activo);
			}
		}
	}

	bool _menor_carga(t_nodo_global* nodo_global1, t_nodo_global* nodo_global2) {
		return nodo_global1->carga_trabajo < nodo_global2->carga_trabajo;
	}

	list_sort(lista_nodos_globales_bloque, (void *) _menor_carga);

	int empty_list = list_is_empty(lista_nodos_globales_bloque);

	if (empty_list) {
		log_error_consola("ERROR - No se encontro nodo activo.");
		ret.nombre = NULL;
	} else {
		t_nodo_global* nodo_global = list_get(lista_nodos_globales_bloque, 0);
		log_debug_consola("Nodo con menor carga: %s, carga: %d",
				nodo_global->nodo.nombre, nodo_global->carga_trabajo);
		nodo_global->carga_trabajo += carga_map;
		ret = nodo_global->nodo;
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
	log_debug_interno("actualizando job, map ok. job: %d, map: %d", socket, id);
	t_job* job_actual = job_por_socket(socket);

	if (!job_actual) {
		log_error_consola("El job no existe. job: %d, map: %d", socket, id);
		exit(1);
	}

	t_map* map_actual = map_por_id(id, job_actual);

	if (!map_actual) {
		log_error_consola("El map no existe. job: %d, map: %d", socket, id);
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


	eliminar_carga_nodo(map_actual->arch_tmp.nodo,carga_map);
	log_debug_interno("job actualizado, map ok. job: %d, map: %d", socket, id);
}

void eliminar_carga_nodo(t_nodo nodo,int carga_operacion){

	pthread_mutex_lock(&mutex_nodos);
	log_debug_consola("eliminando carga de nodo: %s", nodo.nombre);
	bool _nodo_por_nombre(t_nodo_global* nodo_global_actual) {
		return !strcmp(nodo_global_actual->nodo.nombre,nodo.nombre);
	}

	t_nodo_global* nodo_global = list_find(lista_nodos,
			(void*) _nodo_por_nombre);

	if (nodo_global != NULL) {
		nodo_global->carga_trabajo-= carga_operacion;
		log_debug_consola("se elimino carga del nodo: %s. carga actual: %d", nodo.nombre, nodo_global->carga_trabajo);
	} else {
		log_debug_consola("nodo: %s no esta activo", nodo.nombre);
	}

	pthread_mutex_unlock(&mutex_nodos);

}

void elimina_nodo_desconectado(char* nombre_nodo) {
	log_debug_interno("eliminando nodo inactivo. nodo: %s", nombre_nodo);
	pthread_mutex_lock(&mutex_nodos);

	bool _nodo_por_nombre(t_nodo_global* nodo_global_actual) {
		return !strcmp(nodo_global_actual->nodo.nombre, nombre_nodo);
	}

	list_remove_and_destroy_by_condition(lista_nodos, (void*) _nodo_por_nombre,
			(void*) destroy_nodo);

	log_debug_interno(" nodo inactivo eliminado. nodo: %s", nombre_nodo);
	pthread_mutex_unlock(&mutex_nodos);
}

void actualiza_job_map_error(int id, int socket) {
	pthread_mutex_lock(&mutex_jobs);
	log_debug_interno("actualizando job, map error. job: %d, map: %d", socket, id);
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

	log_debug_interno("job actualizado, map error. job: %d, map: %d -> replanificando", socket, id);

	elimina_nodo_desconectado(map_actual->arch_tmp.nodo.nombre);

	void _planifica_map(t_map* map) {
		if (map->estado == PENDIENTE || map->estado == FIN_ERROR) {
			t_temp arch_tmp;
			arch_tmp.nombre = getRandName(map->archivo.nombre,
					string_itoa(map->archivo.bloque));
			arch_tmp.nodo = get_nodo_menos_cargado(map->archivo.copias);

			if(arch_tmp.nodo.nombre == NULL){
				log_error_consola("Error encontrando Nodo Disponible. Job %d Cancelado.", job_actual->socket);
				finalizar_job_a_si_mismo(job_actual);
			}

			map->arch_tmp = arch_tmp;
		}
	}

	ejecuta_maps(job_actual);

	pthread_mutex_unlock(&mutex_jobs);

	eliminar_carga_nodo(map_actual->arch_tmp.nodo,carga_map);
}

void actualiza_job_reduce_ok(int id, int socket) {
	pthread_mutex_lock(&mutex_jobs);
	log_debug_interno("actualizando job, REDUCE ok. job: %d, reduce: %d", socket, id);
	t_job* job_actual = job_por_socket(socket);

	if (!job_actual) {
		log_error_consola("El job no existe. job: %d, reduce: %d", socket, id);
		exit(1);
	}

	t_reduce* reduce_actual = reduce_por_id(id, job_actual);

	if (!reduce_actual) {
		log_error_consola("El reduce no existe. job: %d, reduce: %d", socket, id);
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
	log_debug_interno("job actualizado, REDUCE ok. job: %d, reduce: %d", socket, id);
	eliminar_carga_nodo(reduce_actual->arch_tmp.nodo,carga_reduce);

}

void actualizar_job_reduce_error(int id, int socket, char* nombre_nodo) {
	//TODO: Ejecutar tareas de limpieza por cancelación de job
	log_error_interno("fin reduce error. job: %d, reduce: %d. Cancelando Job", socket, id);
	t_job* job = job_por_socket(socket);
	finalizar_job_hijo(job); 	//TODO eliminar hilo job

	if (!job) {
			log_error_consola("El job no existe. job: %d, reduce: %d", socket, id);
			exit(1);
		}

	t_reduce* reduce_actual = reduce_por_id(id, job);

		if (!reduce_actual) {
			log_error_consola("El reduce no existe. job: %d, reduce: %d", socket, id);
			exit(1);
		}

	eliminar_carga_nodo(reduce_actual->arch_tmp.nodo,carga_reduce);
}

void finalizar_job_hijo(t_job* job){
	pthread_mutex_lock(&mutex_jobs);
	bool _job(t_job* jobLista) {
			return jobLista == job;
		}
	list_remove_by_condition(lista_jobs, (void*)_job);
	pthread_mutex_unlock(&mutex_jobs);

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
