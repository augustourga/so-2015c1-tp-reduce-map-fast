/*
 ============================================================================
 Name        : nuevo.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <utiles/utiles.h>

int HacerMagiaConArchivo(char*);
void realizar_handshake_job();

char* nombre_archivo;
int sock_job;

int main(void) {
	int sock = server_socket(9999);
	sock_job = accept_connection(sock);
	t_msg* mensaje;

	realizar_handshake_job();

	while(1)
	{
		enviar_mensaje(sock_job,mensaje=string_message(EJECUTAR_MAP,"127.0.0.1",1,6545));
		destroy_message(mensaje);
		mensaje = string_message(ARCHIVO_JOB_MAP,nombre_archivo,0);
		enviar_mensaje(sock_job,mensaje);
		mensaje = recibir_mensaje(sock_job);
		print_msg(mensaje);

	}
	destroy_message(mensaje);


	return 0;
}

int HacerMagiaConArchivo(char* arch)
{
	printf("NOMBRE ARCHIVO RECIBIDO:%s/n",arch);
	nombre_archivo= (char*) malloc(strlen(arch)+1);
	strcpy(nombre_archivo,arch);
	return 0;

}

void realizar_handshake_job()
{
	t_msg* mensaje;

	mensaje =recibir_mensaje(sock_job);
	destroy_message(mensaje);
	mensaje = recibir_mensaje(sock_job);
	while(mensaje->header.id!=FIN_ENVIO_ARCH)
	{
		HacerMagiaConArchivo(mensaje->stream);
		destroy_message(mensaje);
		mensaje = recibir_mensaje(sock_job);

	}
	destroy_message(mensaje);
	enviar_mensaje(sock_job,id_message(FIN_ENVIO_ARCH));
	destroy_message(mensaje);

}
