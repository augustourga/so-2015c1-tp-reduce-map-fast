#include<commons/bitarray.h>

typedef struct {
	char* data;
	int size;
} t_bloque;

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
} t_mensaje_reduce;

