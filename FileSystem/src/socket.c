#include "socket.h"

//pthread_mutex_t mutex_recv;

void socket_bind(int listen_socket, struct addrinfo* server_info) {
	if (bind(listen_socket, server_info->ai_addr, server_info->ai_addrlen) < 0) {
		log_error_consola("Falló el bind");
		perror("Falló el bind. Error");
		exit(1);
	}
}

int socket_listen(char* puerto_listen) {

	int listen_socket;

//	pthread_mutex_init(&mutex_recv, NULL);

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

int socket_conectado(int socket) {
	char buf[1];
	int bytes;
	if ((bytes = recv(socket, buf, 1, MSG_PEEK)) == 0) {
		shutdown(socket,2);
		close(socket);
	}
	return bytes;
}
