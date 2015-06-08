#include<commons/bitarray.h>

/*****************ESTRUCTURAS**************/
typedef struct {
	char* data;
	int size;
} t_bloque;

typedef struct{
	char accion;
	t_bitarray* parametros;
} t_Mensaje_marta_job;

typedef struct{
	int bloque; //bloque en el que se aplica
	char ip_nodo[13];
	int puerto_nodo;
} t_Marta_job_map;

typedef struct{
	char codigo; //  'A' Conexión y envío de archivos. 'B' Finalizado correctamente. 'C' Finalizado con error.
	char* archivos; //Todos en un solo string separado por comas
}t_Mensaje_job_marta;

typedef struct {
	int tamanio_ejecutable;
	t_bitarray* ejecutable;
	int numBloque;
	char* nombre_archivo_tmp;
} t_mensaje_map;

typedef struct { //Puede ser reemplazado por t_dictionary o t_list;
	char* nodo;
	char* archivo;
} t_tupla_reduce;

typedef struct {
	int tamanio_ejecutable;
	t_bitarray* ejecutable;
	 t_tupla_reduce* archivos;
	char* nombre_archivo_tmp;
} t_mensaje_reduce;

typedef struct {
	t_bitarray* ejecutable;
	 t_tupla_reduce* archivos;
	char* nombre_archivo_tmp;
} t_tupla_map;

typedef struct {
	int id_nodo;
	bool nodo_nuevo;
} t_aceptacion_nodo;
/****************DEFINICIONES***************/
t_bloque* bloque_create(int size);
t_bloque* serializar_aceptacion_nodo(char* ,char* );
t_aceptacion_nodo* deserializar_aceptacion_nodo(t_bloque* bloque) ;
t_bitarray* serializar_mensaje_marta_job(t_Mensaje_marta_job*);
t_bitarray* serializar_marta_job_map(t_Marta_job_map*);
t_Marta_job_map* deserializar_marta_job_map(t_bitarray*);
t_Mensaje_marta_job* deserializar_mensaje_marta_job(t_bitarray*);
t_bitarray* serializar_mensaje_job_marta(t_Mensaje_job_marta*);
t_Mensaje_job_marta* deserializar_mensaje_job_marta(t_bitarray*);
