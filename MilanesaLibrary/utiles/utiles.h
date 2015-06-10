#ifndef UTILES_H_
#define UTILES_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
//#include <panel/panel.h>
#define handle_error(msj) \
	do{perror(msj);exit(EXIT_FAILURE);} while(0)
#define REG_SIZE 4

/* Funciones Macro */

/*
 * Compara dos numeros y retorna el mínimo.
 */
#define min(n, m) (n < m ? n : m)

/*
 * Compara dos numeros y retorna el máximo.
 */
#define max(n, m) (n > m ? n : m)

/*
 * Sleep en microsegundos.
 */
#define msleep(usecs) usleep(usecs * 1000)

/*
 * Alternativa sin undefined behavior a fflush(STDIN) para usar con scanf().
 */
#define clean_stdin_buffer() scanf("%*[^\n]")

/*
 * RNG. Retorna valores entre 0 y limit.
 */
#define randomize(limit) (rand() % (limit + 1))

/*
 * Divide dos enteros redondeando hacia arriba.
 */
#define divRoundUp(n,m) (n % m == 0 ? n / m : n / m + 1)

/* FIN Funciones Macro */

/****************** IDS DE MENSAJES. ******************/

typedef enum {
	NO_NEW_ID,					/* Valor centinela para evitar la modificación de id en modify_message(). */
	/*************************JOB*******************************/
	EJECUTAR_MAP,
	EJECUTAR_REDUCE,
	FIN_MAP,
	FIN_REDUCE,
	CONEXION_JOB,
	ARCHIVO_JOB_MAP,
	ARCHIVO_JOB_REDUCE,
	FIN_ENVIO_ARCH,
	/***********************NODO******************************/
	NODO_MAP,
	NODO_REDUCE,
	SET_BLOQUE,
	GET_BLOQUE,
	GET_FILE_CONTENT,
	INFO_NODO

} t_msg_id;


/****************** ESTRUCTURAS DE DATOS. ******************/

typedef struct {
	t_msg_id id;
	uint32_t length;
	uint16_t argc;
} __attribute__ ((__packed__)) t_header;

typedef struct {
	t_header header;
	char *stream;
	int32_t *argv;
} __attribute__ ((__packed__)) t_msg;


/****************** FUNCIONES SOCKET. ******************/

/*
 * Crea, vincula y escucha un socket desde un puerto determinado.
 */
int server_socket(uint16_t port);

/*
 * Crea y conecta a una ip:puerto determinado.
 */
int client_socket(char* ip, uint16_t port);

/*
 * Acepta la conexion de un socket.
 */
int accept_connection(int sock_fd);

/*
 * Recibe un t_msg a partir de un socket determinado.
 */
t_msg *recibir_mensaje(int sock_fd);

/*
 * Envia los contenidos de un t_msg a un socket determinado.
 */
int enviar_mensaje(int sock_fd, t_msg *msg);


/****************** FUNCIONES T_MSG. ******************/

/*
 * Crea un t_msg sin argumentos, a partir del id.
 */
t_msg *id_message(t_msg_id id);

/*
 * Crea un t_msg a partir de count argumentos.
 */
t_msg *argv_message(t_msg_id id, uint16_t count, ...);

/*
 * Crea un t_msg a partir de un string y count argumentos.
 */
t_msg *string_message(t_msg_id id, char *message, uint16_t count, ...);

/*
 * Agrega nuevos argumentos a un mensaje (estilo FIFO).
 */
t_msg *modify_message(t_msg_id new_id, t_msg *old_msg, uint16_t new_count, ...);

/*
 * Elimina todos los argumentos existentes de un mensaje y agrega nuevos.
 */
t_msg *remake_message(t_msg_id new_id, t_msg *old_msg, uint16_t new_count, ...);

/*
 * Crea un t_msg a partir de los contenidos de un archivo beso y count argumentos.
 */
t_msg *beso_message(t_msg_id id, char *beso_path, uint16_t count, ...);

/*
 * Crea un t_msg a partir de los contenidos de un tcb y count argumentos.
 */
//t_msg *tcb_message(t_msg_id id, t_hilo *tcb, uint16_t count, ...);

/*
 * Libera los contenidos de un t_msg.
 */
void destroy_message(t_msg *mgs);


/****************** FUNCIONES FILE SYSTEM. ******************/

/*
 * Crea un archivo de size bytes de tamaño.
 */
void create_file(char *path,size_t size);

/*
 * Vacía el archivo indicado por path. Si no existe lo crea.
 */
void clean_file(char *path);

/*
 * Lee un archivo y retorna los primeros size bytes de su contenido.
 */
char* read_file(char *path, size_t size);

/*
 * Si existe, copia el contenido del archivo path en dest.
 */
void memcpy_from_file(char *dest,char *path,size_t size);

/*
 * Elimina los primeros size bytes del archivo path, y los retorna.
 */
char* read_file_and_clean(char *path, size_t size);

/*
 * Lee un archivo y retorna todo su contenido.
 */
char* read_whole_file(char *path);

/*
 * Lee un archivo y retorna todo su contenido, vaciándolo.
 */
char* read_whole_file_and_clean(char *path);

/*
 * Abre el archivo indicado por path (si no existe lo crea) y escribe size bytes de data.
 */
void write_file(char *path,char* data,size_t size);

void* file_get_mapped(char* filename);
/****************** FUNCIONES AUXILIARES. ******************/

/*
 * Genera una nueva secuencia de enteros pseudo-random a retornar por rand().
 */
void seedgen(void);

/*
 * Muestra los contenidos y argumentos de un t_msg.
 */
void print_msg(t_msg *msg);

/*
 * Convierte t_msg_id a string.
 */
char *id_string(t_msg_id id);
char* file_combine(char* f1, char* f2);

void file_mmap_free(char* mapped, char* filename);

void free_null(void** data);
/*
 * Recupera los contenidos de un tcb cargado a mensaje.
 */
//t_hilo *retrieve_tcb(t_msg *msg);

/*
 * Convierte t_cola a string.
 */
//char *string_cola(t_cola cola);


//void print_tcb(t_hilo *tcb);

#endif /* UTILES_H_ */
