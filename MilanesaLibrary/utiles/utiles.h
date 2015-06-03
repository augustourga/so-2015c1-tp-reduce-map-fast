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
//#include <panel/panel.h>

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

	INIT_CONSOLE,				/* Pedido de creación de hilo principal de Consola a Kernel. */
	KILL_CONSOLE,				/* Respuesta de finalización por error de Kernel a Consola. */

	CPU_CONNECT,				/* Pedido de conexión de CPU a Kernel. */

	CPU_TCB,					/* Pedido de TCB de CPU a Kernel. */
	NEXT_TCB,					/* Envío de TCB de Kernel a CPU. */

	RETURN_TCB,					/* Retorno de TCB de CPU a Kernel. */

	FINISHED_THREAD,			/* Envío de TCB, cuya ejecución ha finalizado, de CPU a Kernel. */

	CPU_ABORT,					/* Pedido de la CPU al Kernel para abortar la ejecución de un hilo */

	/****************** SERVICIOS EXPUESTOS A CPU: INTERRUPCIÓN. ******************/ 
	CPU_INTERRUPT,				/* Pedido de interrupción de un hilo de proceso de CPU a Kernel. */

	/****************** SERVICIOS EXPUESTOS A CPU: ENTRADA ESTÁNDAR. ******************/
	NUMERIC_INPUT,				/* Pedido de input numérico de CPU. */
	STRING_INPUT,				/* Pedido de input de string de CPU. */
	REPLY_NUMERIC_INPUT,		/* Respuesta de numeric input de Consola. */
	REPLY_STRING_INPUT,			/* Respuesta de string input de Consola. */

	/****************** SERVICIOS EXPUESTOS A CPU: SALIDA ESTÁNDAR. ******************/
	NUMERIC_OUTPUT,				/* Pedido de output numérico de CPU. */
	STRING_OUTPUT,				/* Pedido de output de string de CPU. */

	/****************** SERVICIOS EXPUESTOS A CPU: CREAR HILO. ******************/
	CPU_CREA,					/* Pedido de nuevo hilo de proceso de CPU a Kernel. */
	CREA_OK,					/* Respuesta de hilo creado. */
	CREA_FAIL,					/* Respuesta de fallo al crear hilo. */

	/****************** SERVICIOS EXPUESTOS A CPU: JOIN. ******************/
	CPU_JOIN,					/* Pedido de unión a hilo de proceso de CPU a Kernel. */

	/****************** SERVICIOS EXPUESTOS A CPU: BLOQUEAR. ******************/
	CPU_BLOCK,					/* Pedido de bloqueo de hilo de proceso por recurso de CPU a Kernel. */

	/****************** SERVICIOS EXPUESTOS A CPU: DESPERTAR. ******************/
	CPU_WAKE,					/* Pedido de desbloqueo hilo de proceso bloqueado por recurso de CPU a Kernel. */


	/****************** INTERFAZ MSP: RESERVAR SEGMENTO. ******************/
	CREATE_SEGMENT,				/* Pedido de creación de segmento a MSP. */
	OK_CREATE,					/* Respuesta de segmento creado de MSP. */
	FULL_MEMORY,				/* Respuesta de memoria llena. */
	INVALID_SEG_SIZE,			/* Respuesta de tamaño de segmento inválido. */
	MAX_SEG_NUM_REACHED,		/* Respuesta de máxima cantidad de segmentos del proceso alcanzada. */

	/****************** INTERFAZ MSP: DESTRUIR SEGMENTO. ******************/
	DESTROY_SEGMENT,			/* Pedido de destrucción de segmento a MSP. */
	OK_DESTROY,					/* Respuesta de eliminado correcto de MSP. */

	/****************** INTERFAZ MSP: SOLICITAR MEMORIA. ******************/
	REQUEST_MEMORY,				/* Pedido de datos a la MSP. */
	OK_REQUEST,					/* Respuesta de lectura correcta de MSP. */

	/****************** INTERFAZ MSP: ESCRIBIR MEMORIA. ******************/
	WRITE_MEMORY,				/* Pedido de escritura en memoria a MSP. */
	OK_WRITE,					/* Respuesta de escritura correcta de MSP. */

	/****************** INTERFAZ MSP: SOLICITAR/ESCRIBIR MEMORIA. ******************/
	INVALID_DIR,				/* Respuesta de dirección inválida. */
	SEGMENTATION_FAULT			/* Respuesta de error de segmento en lectura/escritura de memoria. */

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
