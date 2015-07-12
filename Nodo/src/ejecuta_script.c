#include "ejecuta_script.h"

void error(char *s) {
	perror(s);
	exit(1);
}

int ejecuta_map(char* data, char* path_ejecutable, char* path_salida) {
	log_info_consola("Entrando a Ejecutar...");

	int size = strlen(data);
	int in[2], out[2];
	//Crea 2 pipes, uno para stdin y otro para stdout. in[0] y out[0] se usan para leer e in[1] y out[1] para escribir
	if (pipe(in) < 0) {
		error("pipe in");
		return -1;
	}

	if (pipe(out) < 0) {
		error("pipe in");
		return -1;
	}

	pid_t pid_sort = fork();

	if (!pid_sort) {
		//Fork devuelve 0 en el hijo y el pid del hijo en el padre, así que acá estoy en el hijo.

		//Cierra stdin stdout y stdeer
		close(0);

		//Duplica stdin al lado de lectura in y stdout y stdeer al lado de escritura out
		dup2(in[0], 0);
		freopen(path_salida, "w", stdout);

		//Cierra los pipes que se usan en el padre
		close(in[1]);

		//La imagen del proceso hijo se reemplaza con la del path_ejecutable
		execl("usr/bin/sort", "usr/bin/sort", NULL);
		//On success acá nunca llega porque la imágen (incluido el código) se reemplazó en la lina anterior
		error("No se pudo ejecutar el proceso");
		return -1;
	}

	pid_t pid_map = fork();

	if (!pid_map) {
		//Fork devuelve 0 en el hijo y el pid del hijo en el padre, así que acá estoy en el hijo.

		//Cierra stdin stdout y stdeer
		close(0);
		close(1);

		//Duplica stdin al lado de lectura in y stdout y stdeer al lado de escritura out
		dup2(out[0], 0);
		dup2(in[1], 1);

		//Cierra los pipes que se usan en el padre
		close(in[1]);
		close(out[0]);

		//La imagen del proceso hijo se reemplaza con la del path_ejecutable
		execl(path_ejecutable, path_ejecutable, NULL);
		//On success acá nunca llega porque la imágen (incluido el código) se reemplazó en la lina anterior
		error("No se pudo ejecutar el proceso");
		return -1;
	}

	//Cierra los lados de los pipes que se usan en el hijo
	close(in[0]);
	int total = 0;
	int pendiente = size;
	//Escribe en el lado de escritura del pipe que el proceso hijo va a ver como stdin
	while (total < pendiente) {
		int enviado = write(out[1], data, pendiente);
		total += enviado;
		pendiente -= enviado;
	}

	//Se cierra para generar un EOF y que el proceso hijo termine de leer de stdin
	close(in[1]);
	free(data);
	int status;
	waitpid(pid_sort, &status, 0);
	FILE* temp_file = fopen(path_salida, "r");
	fclose(temp_file);
	return WEXITSTATUS(status);
}

int ejecuta_reduce(char* data, char* path_ejecutable, char* path_salida) {
	log_info_consola("Entrando a Ejecutar...");

	int size = strlen(data);
	int in[2];
	//Crea 2 pipes, uno para stdin y otro para stdout. in[0] y out[0] se usan para leer e in[1] y out[1] para escribir
	if (pipe(in) < 0) {
		error("pipe in");
		return -1;
	}

	pid_t pid = fork();

	if (!pid) {
		//Fork devuelve 0 en el hijo y el pid del hijo en el padre, así que acá estoy en el hijo.

		//Cierra stdin stdout y stdeer
		close(0);

		//Duplica stdin al lado de lectura in y stdout y stdeer al lado de escritura out
		dup2(in[0], 0);
		freopen(path_salida, "w", stdout);

		//Cierra los pipes que se usan en el padre
		close(in[1]);

		//La imagen del proceso hijo se reemplaza con la del path_ejecutable
		execl(path_ejecutable, path_ejecutable, NULL);
		//On success acá nunca llega porque la imágen (incluido el código) se reemplazó en la lina anterior
		error("No se pudo ejecutar el proceso");
		return -1;
	}

	//Cierra los lados de los pipes que se usan en el hijo
	close(in[0]);
	int total = 0;
	int pendiente = size;
	//Escribe en el lado de escritura del pipe que el proceso hijo va a ver como stdin
	while (total < pendiente) {
		int enviado = write(in[1], data, pendiente);
		total += enviado;
		pendiente -= enviado;
	}

	//Se cierra para generar un EOF y que el proceso hijo termine de leer de stdin
	close(in[1]);
	free(data);
	int status;
	waitpid(pid, &status, 0);
	FILE* temp_file = fopen(path_salida, "r");
	fclose(temp_file);
	return WEXITSTATUS(status);
}
char* generar_nombre_rutina(int map_id, char*map_o_reduce, int numeroBloque) {
	char* file_map1 = string_new();
	string_append(&file_map1, "rutina_");

	char str[15];

	sprintf(str, "%d", map_id);
	string_append(&file_map1, str);
	string_append(&file_map1, "_");

	string_append(&file_map1, map_o_reduce);
	string_append(&file_map1, "_");
	sprintf(str, "%d", numeroBloque);
	string_append(&file_map1, str);

	string_append(&file_map1, ".txt");
	return file_map1;
}

char* generar_nombre_temporal(int map_id, char*map, int numeroBloque) {
	char* file_map1 = string_new();
	string_append(&file_map1, "tmp_");

	char str[15];

	sprintf(str, "%d", map_id);
	string_append(&file_map1, str);
	string_append(&file_map1, "_");

	string_append(&file_map1, map);
	string_append(&file_map1, "_");
	sprintf(str, "%d", numeroBloque);
	string_append(&file_map1, str);

	string_append(&file_map1, ".txt");
	return file_map1;
}

