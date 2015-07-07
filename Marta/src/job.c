#include "job.h"

pthread_mutex_t mutex_id_job = PTHREAD_MUTEX_INITIALIZER;

t_job* job_crear() {
	t_job* job = malloc(sizeof(t_job));
	job->maps = list_create();
	job->reduces = list_create();
	sem_init(&job->sem_maps_fin, 0, 0);
	sem_init(&job->sem_reduces_fin, 0, 0);
	job->replanifica = false;
	return job;
}

void generar_maps(t_job** job, char* ruta_mdfs) {

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

		list_add((*job)->maps, map);

		bloque_archivo++;
	}

	string_iterate_lines(bloques_archivo, (void*) _crear_map);

}

void ejecutar_maps(t_job* job) {

	while (true) {

		void ejecutar_map(t_map* map) {
			char* stream = string_duplicate(map->arch_tmp.nodo.ip);
			string_append(&stream, "|");
			string_append(&stream, string_duplicate(map->arch_tmp.nombre));
			t_msg* message = string_message(EJECUTAR_MAP, stream, 3, map->arch_tmp.nodo.puerto, map->id, map->arch_tmp.nodo.numero_bloque);
			map->estado = EN_EJECUCION;
			enviar_mensaje(job->socket, message);
			destroy_message(message);
		}

		list_iterate(job->maps, (void*) ejecutar_map);

		sem_wait(&job->sem_maps_fin);
		if (!job->replanifica) {
			break;
		}
	}
}

void ejecutar_reduce_final(t_job* job) {

	t_reduce* reduce = job->reduce_final;

	char* stream = string_duplicate(reduce->arch_tmp.nodo.ip);
	string_append(&stream, "|");
	string_append(&stream, string_duplicate(reduce->arch_tmp.nombre));
	t_msg* message = string_message(EJECUTAR_REDUCE, stream, 2, reduce->arch_tmp.nodo.puerto, reduce->id);
	reduce->estado = EN_EJECUCION;
	enviar_mensaje(job->socket, message);

	void _genera_mensaje(t_temp* temp) {
		stream = string_duplicate(temp->nodo.ip);
		string_append(&stream, "|");
		string_append(&stream, string_duplicate(temp->nombre));
		t_msg* message = string_message(ARCHIVOS_NODO_REDUCE, stream, 1, reduce->arch_tmp.nodo.puerto);
		enviar_mensaje(job->socket, message);
	}

	list_iterate(reduce->temporales, (void*) _genera_mensaje);
	message = id_message(FIN_ENVIO_MENSAJE);
	enviar_mensaje(job->socket, message);

	destroy_message(message);

}

void ejecutar_reduces(t_job* job) {

	while (true) {

		void ejecutar_reduce(t_reduce* reduce) {
			char* stream = string_duplicate(reduce->arch_tmp.nodo.ip);
			string_append(&stream, "|");
			string_append(&stream, string_duplicate(reduce->arch_tmp.nombre));
			t_msg* message = string_message(EJECUTAR_REDUCE, stream, 2, reduce->arch_tmp.nodo.puerto, reduce->id);
			reduce->estado = EN_EJECUCION;
			enviar_mensaje(job->socket, message);

			void _genera_mensaje(t_temp* temp) {
				stream = string_duplicate(temp->nodo.ip);
				string_append(&stream, "|");
				string_append(&stream, string_duplicate(temp->nombre));
				t_msg* message = string_message(ARCHIVOS_NODO_REDUCE, stream, 1, reduce->arch_tmp.nodo.puerto);
				enviar_mensaje(job->socket, message);
			}

			list_iterate(reduce->temporales, (void*) _genera_mensaje);
			message = id_message(FIN_ENVIO_MENSAJE);
			enviar_mensaje(job->socket, message);

			destroy_message(message);
		}

		list_iterate(job->reduces, (void*) ejecutar_reduce);

		sem_wait(&job->sem_reduces_fin);
		if (!job->replanifica) {
			break;
		}
	}
}
