#include "job.h"

pthread_mutex_t mutex_id_job = PTHREAD_MUTEX_INITIALIZER;

t_job* job_crear() {
	t_job* job = malloc(sizeof(t_job));
	job->maps = list_create();
	job->reduces = list_create();
	sem_init(&job->sem_maps_fin, 0, 0);
	sem_init(&job->sem_reduces_fin, 0, 0);
	log_debug_consola("Se creo con exito el job. id: %d", job->socket);
	return job;
}

void ejecuta_maps(t_job* job) {

	void _ejecuta_map(t_map* map) {
		if (map->estado == PENDIENTE || map->estado == FIN_ERROR) {
			char* stream = string_duplicate(map->arch_tmp.nodo.ip);
			string_append(&stream, "|");
			string_append(&stream, map->arch_tmp.nodo.nombre);
			string_append(&stream, "|");
			string_append(&stream, string_duplicate(map->arch_tmp.nombre));
			t_msg* message = string_message(EJECUTAR_MAP, stream, 3, map->arch_tmp.nodo.puerto, map->id, map->arch_tmp.nodo.numero_bloque);
			map->estado = EN_EJECUCION;
			enviar_mensaje(job->socket, message);
			log_info_interno("Enviando MAP. job: %d, id_operacion: %d, ip_nodo: %s,puerto_nodo: %d, bloque: %d,nombre_temp: %s", job->socket, map->id,
					map->arch_tmp.nodo.ip, map->arch_tmp.nodo.puerto, map->arch_tmp.nodo.numero_bloque, map->arch_tmp.nombre);
			destroy_message(message);
		}
	}
	log_debug_consola("creando map threads. Job: %d", job->socket);
	list_iterate(job->maps, (void*) _ejecuta_map);
	log_debug_consola("map threads creados. Job: %d", job->socket);
}

void ejecuta_reduce(t_job* job, t_reduce* reduce) {
	char* stream = string_duplicate(reduce->arch_tmp.nodo.ip);
	string_append(&stream, "|");
	string_append(&stream, reduce->arch_tmp.nodo.nombre);
	string_append(&stream, "|");
	string_append(&stream, string_duplicate(reduce->arch_tmp.nombre));
	t_msg* message = string_message(EJECUTAR_REDUCE, stream, 2, reduce->arch_tmp.nodo.puerto, reduce->id);
	reduce->estado = EN_EJECUCION;
	enviar_mensaje(job->socket, message);

	void _genera_mensaje(t_temp* temp) {
		stream = string_duplicate(temp->nodo.ip);
		string_append(&stream, "|");
		string_append(&stream, temp->nodo.nombre);
		string_append(&stream, "|");
		string_append(&stream, string_duplicate(temp->nombre));
		t_msg* message = string_message(ARCHIVOS_NODO_REDUCE, stream, 1, temp->nodo.puerto);
		enviar_mensaje(job->socket, message);
	}

	list_iterate(reduce->temporales, (void*) _genera_mensaje);
	message = id_message(FIN_ENVIO_MENSAJE);
	enviar_mensaje(job->socket, message);

	destroy_message(message);
}

void ejecuta_reduce_final(t_job* job) {

	log_debug_interno("creando reduce final thread. job: %d", job->socket);
	t_reduce* reduce = job->reduce_final;
	ejecuta_reduce(job, reduce);
	log_debug_interno("Reduce final thread creado. job: %d", job->socket);

}

void ejecuta_reduces_parciales(t_job* job) {

	void _ejecuta_reduce(t_reduce* reduce) {
		ejecuta_reduce(job, reduce);
	}
	log_debug_interno("creando reduce parciales threads. job: %d", job->socket);
	list_iterate(job->reduces, (void*) _ejecuta_reduce);
	log_debug_interno("reduces threads creados. job: %d", job->socket);
}
