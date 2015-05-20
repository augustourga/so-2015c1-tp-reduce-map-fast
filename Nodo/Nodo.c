/*
 * Nodo.c
 *
 *  Created on: 25/4/2015
 *      Author: utnso
 */
#include "Nodo.h"

int main (){


Log_Nodo = log_create(LOG_FILE,PROCESO,1,LOG_LEVEL_TRACE);


   t_bloque* bloquesito;
   bloquesito = serializar_aceptacion_nodo(NODO_NUEVO,NOMBRE_NODO);
   	  deserializar_aceptacion_nodo(bloquesito);

if(levantarConfiguracionNodo())
{ log_error(Log_Nodo,"Hubo errores en la carga de las configuraciones.");
	}

if(levantarHiloFile()){ log_error(Log_Nodo,"Conexion con File System fallida.");
}

if(levantarServer()){ log_error(Log_Nodo,"Error al levantar el Server.");
}


return 0;
}

int levantarConfiguracionNodo(){


		t_config* archivo_config = config_create(PATH);

		PUERTO_FS = config_get_int_value(archivo_config, "PUERTO_FS");
		IP_FS = strdup(config_get_string_value(archivo_config, "IP_FS"));
		ARCHIVO_BIN= strdup(config_get_string_value(archivo_config, "ARCHIVO_BIN"));
		DIR_TEMP= strdup(config_get_string_value(archivo_config, "DIR_TEMP"));
		NODO_NUEVO=strdup(config_get_string_value(archivo_config, "NODO_NUEVO"));
		IP_NODO = strdup(config_get_string_value(archivo_config, "IP_NODO"));
		PUERTO_NODO = config_get_int_value(archivo_config, "PUERTO_NODO");

		config_destroy(archivo_config);

return 0;
}

int levantarHiloFile(){
	pthread_t thr_Conexion_FileSystem;
	t_conexion_nodo* reg_conexion = malloc(sizeof(t_conexion_nodo));
   t_bloque* bloquesito;
   reg_conexion->sock_fs= obtener_socket();
    rcx = pthread_create(&thr_Conexion_FileSystem, NULL,(void *)
    conectarFileSystem, reg_conexion);
		if (rcx != 0) {
				log_error(Log_Nodo,"El thread que acepta las conexiones entrantes no pudo ser creado.");
			} else {

	 bloquesito = serializar_aceptacion_nodo(NODO_NUEVO,NOMBRE_NODO);
	  deserializar_aceptacion_nodo(bloquesito);
//              send(reg_conexion->sock_fs,bloquesito->data,bloquesito->size,0);
			}

/*
    conectarFileSystem(reg_conexion);
	*/
			return 0;
}

void conectarFileSystem(t_conexion_nodo* reg_conexion){
	 	conectar_socket(PUERTO_FS,IP_FS,reg_conexion->sock_fs);
	 	puts("conectado al File System");

}
int levantarServer(){

	fd_set read_fs; // descriptores q estan lisots para leer
	fd_set master; //descriptores q q estan actualemnte conectados
	size_t tamanio; // hace positivo a la variable
	int socketEscucha, socketNuevaConexion;
	int nbytesRecibidos;
	int max;
	struct sockaddr_in cliente;

	FD_ZERO(&master);
	FD_ZERO(&read_fs);
	char *buffer=malloc(100*sizeof(char));
	socketEscucha= obtener_socket();
    vincular_socket(socketEscucha,PUERTO_NODO);
    if (listen(socketEscucha, 10) != 0) {
    			perror("Error al poner a escuchar socket");
    			return EXIT_FAILURE;
    		}

    	printf("Escuchando conexiones entrantes.\n");
// Escuchar nuevas conexiones entrantes.
//arranca select

	FD_SET(socketEscucha,&master);
	max = socketEscucha;
	printf("%d \n",socketEscucha);

	while(1){

		memcpy(&read_fs, &master, sizeof(master));
		int dev_select;
		if( (dev_select = select(max +1, &read_fs,NULL,NULL,NULL))==-1){
			perror("select");

		}
		//printf("select = %d \n",dev_select);
		int i;
		for(i=0; i<= max ;i++) //max : cantidad max de sockets
				{
			if(FD_ISSET(i,&read_fs)){
			//  printf("i = %d \n max = %d \n",i,max);
			 if(i==socketEscucha){
			// pasar a una funcion generica aceptar();
			tamanio = sizeof(struct sockaddr_in);
			if ((socketNuevaConexion = accept(socketEscucha, (struct sockaddr*)&cliente,&tamanio)) < 0){
			  perror("Error al aceptar conexion entrante");
			  return EXIT_FAILURE;
			 }
				else{	if(socketNuevaConexion >max){
				max =socketNuevaConexion;
						 		}
				FD_SET(socketNuevaConexion,&master);
				//printf("nueva conexion de %s desde socket %d \n",inet_ntoa(cliente.sin_addr), socketNuevaConexion);
								}//if del accept. Recibir hasta BUFF_SIZE datos y almacenarlos en 'buffer'.
					}else{

						//verifica si esta en el cojunto de listos para leer
						//pasarlo a una funcion generica
						if ((nbytesRecibidos = recv(i, buffer, BUFF_SIZE,0)) > 0) {
						     int offset = 0, tmp_size = 0, code ;
							 memcpy(&code, buffer + offset, tmp_size = sizeof(code));
							 offset += tmp_size;
							 switch(code){
                         	   case '1':
							     	   printf("getBloque()");
/*											getBloque(numero) devovera el contenido del bloque "20*numero"
											almacenado en el espacio de datos.
											contenidoDeBloque getBloque(unNumero);
*/                             break;
			     	     	   case '2':
							     	   printf("setBloque()");
/*
											setBloque almacenara los "datos" en "20*numero"
											setBloque(numero,datos);
*/							   break;
							   case '3':
								     printf("getFileContent()");
 											/*
											 arch getFileContent(char* nombre) devolvera el contenido
											* del archivo "nombre.dat" almacenado en el espacio temporal
											getFileContent(nombre);
*/   						   break;
							   case '4':
								     printf("ejecutar_mapping()");
/*
											ejecutar_mapping(ejecutable,num_bloque,nombre_archivo);
*/
							   break;
							   case '5':
								     printf("ejecutar_Reduce()");
/*
											ejecutar_reduce(ejecutable,lista_archivos,nombre_archivo_tmp);
							 */
								     break;
   							printf("Mensaje recibido de socket %d: ",i);
							fwrite(buffer, 1, nbytesRecibidos, stdout);
							printf("\n");
							printf("Tamanio del buffer %d bytes!\n", nbytesRecibidos);
							fflush(stdout);

							     	   }
						}
						else if(nbytesRecibidos == 0) {
							printf("se desconecto el socket %d \n",i);
							FD_CLR(i,&master);
							// aca se tendria q actualizar los maximos.

						}else
						{
							printf("Error al recibir datos\n i= %d\n",i);
							break;

					}
				}//1er if
				}// for
				}




				}
	close(socketEscucha);
	return EXIT_SUCCESS;

}




