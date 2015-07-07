#include "tarea.h"

bool replanifica = false;
pthread_mutex_t mutex_id_tarea = PTHREAD_MUTEX_INITIALIZER;

int id_tarea_next() {
	static int id_tarea = 0;
	pthread_mutex_lock(&mutex_id_tarea);
	id_tarea++;
	pthread_mutex_unlock(&mutex_id_tarea);
	return id_tarea;
}

t_map* map_crear() {
	t_map* map = malloc(sizeof(t_map));
	map->id = id_tarea_next();
	map->estado = PENDIENTE;
	return map;
}

t_reduce* reduce_crear() {
	t_reduce* reduce = malloc(sizeof(t_reduce));
	reduce->id = id_tarea_next();
	reduce->temporales = list_create();
	reduce->estado = PENDIENTE;
	return reduce;
}
