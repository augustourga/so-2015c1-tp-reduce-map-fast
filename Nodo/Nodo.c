/*
 * Nodo.c
 *
 *  Created on: 25/4/2015
 *      Author: utnso
 */
#include "Nodo.h"

int main (){

levantarConfiguracionNodo();
//crearEspacioDatos();
conectarFileSystem();

printf("puerto %d,ip fs %s",PUERTO_FS,IP_FS);
/*getBloque(numero) devovera el contenido del bloque "20*numero"
 almacenado en el espacio de datos.

contenidoDeBloque getBloque(unNumero);
*/
/* setBloque almacenara los "datos" en "20*numero"

 setBloque(numero,datos);*/
/* arch getFileContent(char* nombre) devolvera el contenido
 * del archivo "nombre.dat" almacenado en el espacio temporal


getFileContent(nombre);
*/
return 0;
}

void levantarConfiguracionNodo(){


		t_config* archivo_config = config_create(PATH);

		PUERTO_FS = config_get_int_value(archivo_config, "PUERTO_FS");
		IP_FS = strdup(config_get_string_value(archivo_config, "IP_FS"));
		ARCHIVO_BIN= strdup(config_get_string_value(archivo_config, "ARCHIVO_BIN"));
		DIR_TEMP= strdup(config_get_string_value(archivo_config, "DIR_TEMP"));
		NODO_NUEVO=strdup(config_get_string_value(archivo_config, "NODO_NUEVO"));
		IP_NODO = strdup(config_get_string_value(archivo_config, "IP_NODO"));
		PUERTO_NODO = config_get_int_value(archivo_config, "PUERTO_NODO");

		config_destroy(archivo_config);


}

void conectarFileSystem(){
	t_conexion_nodo* reg_conexion = malloc(sizeof(t_conexion_nodo));

	     reg_conexion->sock_fs= obtener_socket();

	 	conectar_socket(PUERTO_FS,IP_FS,reg_conexion->sock_fs);
	 	puts("conectado al File System");

}
