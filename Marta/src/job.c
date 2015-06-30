#include "job.h"

bool replanificar = false;

void procesar_job(void* argumentos) {

	struct arg_job* args = argumentos;

	int socket_job = args->socket;

	char** datos = string_split(args->mensaje->stream, "|");

	char* archivo_final = datos[0];
	int i;
	for (i = 1; *datos[i + 1] == ! NULL; i++) {
		generar_lista_bloques(datos[i]);
	}
	bool combiner = datos[i];

	ejecutar_tareas(combiner, archivo_final, socket_job);

}

void ejecutar_tareas(bool combiner, char* archivo_final, int socket_job) {


	while (reduces_pendientes(socket_job)) {

		if (maps_pendientes(socket_job)) {
			ejecutar_maps(socket_job);
		}
		bool puedo_ejecutar_reduce = maps_finalizados(socket_job) && reduces_pendientes(socket_job);

		if (puedo_ejecutar_reduce) {
			ejecutar_reduces(combiner, socket_job);
		}

	}
	copiar_archivo_final();

}

bool reduces_pendientes(int socket){
	return true; //TODO: reduces pendientes
}

bool maps_pendientes(int socket){
	return true; //TODO: maps pendientes
}

bool maps_finalizados(int socket){
	return true; //TODO: maps finalizados
}
