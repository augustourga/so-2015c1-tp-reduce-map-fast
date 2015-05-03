/*
 * sockets.c
 *
 *  Created on: 19/09/2014
 *      Author: utnso
 */

#include "sockets.h"

int obtener_socket() {
	int unSocket;
	int optval = 1;

	// Crear un socket:
	// AF_INET: Socket de internet IPv4
	// SOCK_STREAM: Orientado a la conexion, TCP
	// 0: Usar protocolo por defecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	if ((unSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error al crear socket");
		return EXIT_FAILURE;
	}

	// Hacer que el SO libere el puerto inmediatamente luego de cerrar el socket.
	setsockopt(unSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	return unSocket;
}

void conectar_socket(int puerto, char* direccion, int socket) {

	struct sockaddr_in socketInfo;
	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = inet_addr(direccion); //IP
	socketInfo.sin_port = htons(puerto);

	// Conectar el socket con la direccion 'socketInfo'.
	if (connect(socket, (struct sockaddr*) &socketInfo, sizeof(socketInfo))
			< 0) {
		perror("Error al conectar socket");
		exit(0);
	} else {
		printf("CONECTADO");
	}
}

void vincular_socket(int socket, int puerto) {
	// AF_INET: Socket de internet IPv4
	// SOCK_STREAM: Orientado a la conexion, TCP
	// 0: Usar protocolo por defecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	struct sockaddr_in socketInfo;

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = INADDR_ANY; //Notar que aca no se usa inet_addr()
	socketInfo.sin_port = htons(puerto);

	// Vincular el socket con una direccion de red almacenada en 'socketInfo'.
	if (bind(socket, (struct sockaddr*) &socketInfo, sizeof(socketInfo)) != 0) {
		perror("Error al bindear socket escucha");
	}
}

int crear_socket_listen(int puerto) {
	int socketEscucha;
	int optval = 1;
	// Crear un socket:
	// AF_INET: Socket de internet IPv4
	// SOCK_STREAM: Orientado a la conexion, TCP
	// 0: Usar protocolo por defecto para AF_INET-SOCK_STREAM: Protocolo TCP/IPv4
	struct sockaddr_in socketInfo;
	if ((socketEscucha = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return EXIT_FAILURE;
	}

	// Hacer que el SO libere el puerto inmediatamente luego de cerrar el socket.
	setsockopt(socketEscucha, SOL_SOCKET, SO_REUSEADDR, &optval,
			sizeof(optval));

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = INADDR_ANY; //Notar que aca no se usa inet_addr()
	socketInfo.sin_port = htons(puerto);

	// Vincular el socket con una direccion de red almacenada en 'socketInfo'.
	if (bind(socketEscucha, (struct sockaddr*) &socketInfo, sizeof(socketInfo))
			!= 0) {
		perror("Error al bindear socket escucha");
		return EXIT_FAILURE;
	}

	return socketEscucha;
}
