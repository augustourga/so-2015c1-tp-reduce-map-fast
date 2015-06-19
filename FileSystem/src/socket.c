#include "socket.h"

pthread_mutex_t mutex_recv;

void socket_bind(int listen_socket, struct addrinfo* server_info) {
	if (bind(listen_socket, server_info->ai_addr, server_info->ai_addrlen) < 0) {
		log_error_consola("Falló el bind");
		perror("Falló el bind. Error");
		exit(1);
	}
}

int socket_listen(char* puerto_listen) {

	int listen_socket;

	pthread_mutex_init(&mutex_recv, NULL);

	int yes = 1; //Ver si se puede reemplazar por algo más copado

	struct addrinfo hints, *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(NULL, puerto_listen, &hints, &server_info);

	listen_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	// Para evitar el error "address already in use"
	setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

	socket_bind(listen_socket, server_info);

	freeaddrinfo(server_info); // all done with this

	if (listen(listen_socket, BACKLOG) == -1) {
		log_error_consola("Falló el listen");
		perror("Falló el listen. Error");
		exit(2);
	}
	return listen_socket;
}

t_paquete* socket_recibir(int socket, int* cant_bytes) {
	char* largo_char = malloc(sizeof(int));
	t_paquete* mensaje;
	pthread_mutex_lock(&mutex_recv);
	if (((*cant_bytes) = recv(socket, largo_char, sizeof(int), 0)) < 0) {
		log_error_consola("Falló el recv");
		perror("Falló el recv. Error");
		close(socket);
	} else {
		int bytes_a_leer;
		memcpy(&bytes_a_leer, largo_char, sizeof(int));
		mensaje = socket_recv_all(socket, cant_bytes, bytes_a_leer);
	}
	pthread_mutex_unlock(&mutex_recv);
	return mensaje;
}

int socket_conectado(int socket) {
	char buf[1];
	int bytes;
	if ((bytes = recv(socket, buf, 1, MSG_PEEK)) == 0) {
		close(socket);
	}
	return bytes;
}

t_paquete* socket_recv_all(int socket, int* cant_bytes, int largo_mensaje) {
	int bytes_parciales;
	int bytes_restantes = largo_mensaje;
	(*cant_bytes) = 0;
	t_paquete* paquete = NULL;
	char* mensaje = malloc(largo_mensaje);
	while ((*cant_bytes) < largo_mensaje) {
		bytes_parciales = recv(socket, mensaje, bytes_restantes, 0);
		if (bytes_parciales <= 0) {
			return paquete;
		}
		(*cant_bytes) += bytes_parciales;
		bytes_restantes -= bytes_parciales;
	}

	paquete = paquete_deserializar_mensaje(mensaje);

	return paquete;
}

int socket_send_all(int socket, t_paquete* paquete) {
	int cant_bytes = 0;
	int bytes_parciales;
	int largo_mensaje;

	char* mensaje = paquete_serializar_mensaje(paquete, &largo_mensaje);

	int bytes_restantes = largo_mensaje;

	while (cant_bytes < largo_mensaje) {
		bytes_parciales = send(socket, mensaje, bytes_restantes, 0);
		if (bytes_parciales <= 0) {
			return 1;
		}
		cant_bytes += bytes_parciales;
		bytes_restantes -= bytes_parciales;
	}
	return cant_bytes;
}
