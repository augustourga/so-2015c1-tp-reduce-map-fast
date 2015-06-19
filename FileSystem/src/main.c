#include "filesystem.h"
#include "consola.h"
#include "database.h"
#include "server.h"
#include "config.h"
#include <pthread.h>

#define RUTA_LOG "files/log"

struct arg_struct {
	char* puerto_listen;
};

int main(int argc, char* argv[]) {
	char* puerto_listen;
	char* cantidad_nodos;

	log_crear(argv[1], RUTA_LOG, "FileSystem");

	leer_archivo_configuracion(&puerto_listen, &cantidad_nodos);

	struct arg_struct args;
	args.puerto_listen = puerto_listen;

	int cantidad_nodos_minima = strtol(cantidad_nodos, NULL, 10);

	inicializar_filesystem(false, cantidad_nodos_minima);

	pthread_t th_server;
	pthread_create(&th_server, NULL, (void *) iniciar_server, (void*) &args);

	pthread_t th_consola;
	pthread_create(&th_consola, NULL, (void *) iniciar_consola, NULL);

	pthread_join(th_consola, NULL);

	return 0;
}
