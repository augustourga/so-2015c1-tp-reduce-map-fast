#include "sockets.h"

int server_socket(uint16_t port) {
	int sock_fd, optval = 1;
	struct sockaddr_in servername;

	log_debug_consola("Creando socket de escucha");
	/* Create the socket. */
	sock_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		log_error_consola("Error al crear el socket");
		perror("socket");
		return -1;
	}

	/* Set socket options. */
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
		log_error_consola("Error al setear las opciones del socket");
		perror("setsockopt");
		return -2;
	}

	/* Fill ip / port info. */
	servername.sin_family = AF_INET;
	servername.sin_addr.s_addr = htonl(INADDR_ANY);
	servername.sin_port = htons(port);

	/* Give the socket a name. */
	if (bind(sock_fd, (struct sockaddr *) &servername, sizeof(servername)) < 0) {
		log_error_consola("Error al setearle nombre al Socket");
		perror("bind");
		return -3;
	}

	/* Listen to incoming connections. */
	if (listen(sock_fd, 1) < 0) {
		log_error_consola("Error en el listen de conexiones entrantes.");
		perror("listen");
		return -4;
	}

	log_debug_consola("socket de escucha creado con exito.");
	return sock_fd;
}

int client_socket(char *ip, uint16_t port) {
	int sock_fd;
	struct sockaddr_in servername;

	/* Create the socket. */
	sock_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		log_error_consola("Error creando socket.");
		perror("socket");
		return -1;
	}

	/* Fill server ip / port info. */
	servername.sin_family = AF_INET;
	servername.sin_addr.s_addr = inet_addr(ip);
	servername.sin_port = htons(port);
	memset(&(servername.sin_zero), 0, 8);

	/* Connect to the server. */
	if (connect(sock_fd, (struct sockaddr *) &servername, sizeof(servername)) < 0) {
		log_error_consola("Error intentando al conectar al servidor.");
		perror("connect");
		return -2;
	}

	return sock_fd;
}

int accept_connection(int sock_fd) {
	struct sockaddr_in clientname;
	size_t size = sizeof(clientname);
	int new_fd = accept(sock_fd, (struct sockaddr *) &clientname, (socklen_t *) &size);
	if (new_fd < 0) {
		log_error_consola("Error al aceptar nueva conexion");
		perror("accept");
		return -1;
	}
	log_info_interno("Nueva conexion aceptada con exito. id:%d",new_fd);
	return new_fd;
}

void make_socket_non_blocking(int sfd) {
	int flags = fcntl(sfd, F_GETFL, 0);
	if (flags == -1) {
		log_error_consola("Error en la funcion: fcntl");
		perror("fcntl");
		exit(EXIT_FAILURE);
	}

	flags |= O_NONBLOCK;

	if (fcntl(sfd, F_SETFL, flags) == -1) {
		log_error_consola("Error en la funcion fcntl");
		perror("fcntl");
		exit(EXIT_FAILURE);
	}
}

