#include "nodo.h"

pthread_mutex_t mutex_asignacion_bloque = PTHREAD_MUTEX_INITIALIZER;

t_nodo* nodo_crear() {
	t_nodo* nodo = malloc(sizeof(t_nodo));
	memset(nodo->nombre, 0, 80);
	nodo->socket = 0;
	nodo->cantidad_bloques_totales = 0;
	nodo->cantidad_bloques_libres = 0;
	memset(nodo->ip, 0, 16);
	nodo->puerto = 0;
	pthread_rwlock_init(&nodo->lock, NULL);
	return nodo;
}

void nodo_set_nombre(t_nodo* nodo, char* nombre) {
	strcpy(nodo->nombre, nombre);
}

void nodo_eliminar(t_nodo* nodo) {
	free(nodo->bloques);
	free(nodo);
}

void nodo_set_socket(t_nodo* nodo, int socket) {
	nodo->socket = socket;
}

void nodo_set_cantidad_bloques_totales(t_nodo* nodo, int cant_bloques) {
	nodo->cantidad_bloques_totales = cant_bloques;
}

void nodo_set_cantidad_bloques_libres(t_nodo* nodo, int cant_bloques) {
	nodo->cantidad_bloques_libres = cant_bloques;
}

int nodo_bloque_disponible(t_nodo* nodo) {
	int numero_bloque = 0;

	while (nodo->bloques[numero_bloque] == 1) {
		numero_bloque++;
	}
	return numero_bloque;
}

int nodo_asignar_bloque_disponible(t_nodo* nodo) {

	int bloque_disponible = nodo_bloque_disponible(nodo);

	nodo->bloques[bloque_disponible] = 1;
	nodo->cantidad_bloques_libres--;

	return bloque_disponible;
}

bool nodo_lleno(t_nodo* nodo) {
	return nodo->cantidad_bloques_libres == 0;
}

bool nodo_con_espacio(t_nodo* nodo) {
	return !nodo_lleno(nodo);
}

char* nodo_serializar_db(t_nodo* nodo, int* bytes_serializados) {

	(*bytes_serializados) = 0;

	char *nodo_serializado = malloc(sizeof(t_nodo) - sizeof(pthread_rwlock_t) + nodo->cantidad_bloques_totales * sizeof(int));

	paquete_serializar(nodo_serializado, nodo->nombre, sizeof(nodo->nombre), bytes_serializados);

	paquete_serializar(nodo_serializado, &nodo->cantidad_bloques_totales, sizeof(nodo->cantidad_bloques_totales), bytes_serializados);

	paquete_serializar(nodo_serializado, &nodo->cantidad_bloques_libres, sizeof(nodo->cantidad_bloques_libres), bytes_serializados);

	paquete_serializar(nodo_serializado, nodo->bloques, nodo->cantidad_bloques_totales * sizeof(int), bytes_serializados);

	return nodo_serializado;
}

t_nodo* nodo_deserealizar_db(char* nodo_serializado) {

	t_nodo* nodo = nodo_crear();

	int offset = 0;

	paquete_deserializar(nodo->nombre, nodo_serializado, sizeof(nodo->nombre), &offset);

	paquete_deserializar(&nodo->cantidad_bloques_totales, nodo_serializado, sizeof(nodo->cantidad_bloques_totales), &offset);

	paquete_deserializar(&nodo->cantidad_bloques_libres, nodo_serializado, sizeof(nodo->cantidad_bloques_libres), &offset);

	nodo->bloques = malloc(nodo->cantidad_bloques_totales * sizeof(int));
	paquete_deserializar(nodo->bloques, nodo_serializado, nodo->cantidad_bloques_totales * sizeof(int), &offset);

	return nodo;
}

t_nodo* nodo_deserealizar_socket(t_msg* mensaje, int socket) {

	t_nodo* nodo = nodo_crear();

	socklen_t len;
	struct sockaddr_storage addr;
	char ipstr[16];
	int port;

	strcpy(nodo->nombre, mensaje->stream);
	nodo->cantidad_bloques_totales = mensaje->argv[0];

	len = sizeof addr;
	// Obtiene IP y PUERTO del socket del NODO
	getpeername(socket, (struct sockaddr*) &addr, &len);
	if (addr.ss_family == AF_INET) {
		struct sockaddr_in *aux = (struct sockaddr_in *) &addr;
		port = ntohs(aux->sin_port);
		inet_ntop(AF_INET, &aux->sin_addr, ipstr, sizeof ipstr);
	} else { // AF_INET6
		struct sockaddr_in6 *aux = (struct sockaddr_in6 *) &addr;
		port = ntohs(aux->sin6_port);
		inet_ntop(AF_INET6, &aux->sin6_addr, ipstr, sizeof ipstr);
	}

	strcpy(nodo->ip, ipstr);
	nodo->puerto = port;
	nodo->cantidad_bloques_libres = nodo->cantidad_bloques_totales;
	nodo->bloques = calloc(nodo->cantidad_bloques_totales, sizeof(int));
	nodo->socket = socket;

	return nodo;
}

void log_info_nodo_conectado_aceptado(t_nodo* nodo) {
	int bloques_ocupados = nodo->cantidad_bloques_totales - nodo->cantidad_bloques_libres;
	log_info_interno("Se reconectó el nodo %s: BT: %d - BL: %d - BO: %d - Conexión: %s:%d", nodo->nombre, nodo->cantidad_bloques_totales,
			nodo->cantidad_bloques_libres, bloques_ocupados, nodo->ip, nodo->puerto);
}

void log_info_nodo_conectado_nuevo(t_nodo* nodo) {
	int bloques_ocupados = nodo->cantidad_bloques_totales - nodo->cantidad_bloques_libres;
	log_info_interno("Se conectó el nodo nuevo %s: BT: %d - BL: %d - BO: %d - Conexión: %s:%d", nodo->nombre, nodo->cantidad_bloques_totales,
			nodo->cantidad_bloques_libres, bloques_ocupados, nodo->ip, nodo->puerto);
}

void log_info_nodo_desconectado(t_nodo* nodo) {
	int bloques_ocupados = nodo->cantidad_bloques_totales - nodo->cantidad_bloques_libres;
	log_info_interno("Se desconectó el nodo %s: BT: %d - BL: %d - BO: %d - Conexión: %s:%d", nodo->nombre, nodo->cantidad_bloques_totales,
			nodo->cantidad_bloques_libres, bloques_ocupados, nodo->ip, nodo->puerto);
}
