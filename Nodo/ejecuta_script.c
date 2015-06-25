#include "ejecuta_script.h"

void error(char *s) {
	perror(s);
	exit(1);
}

void ejecutar(char* data, char* path_ejecutable, char* path_salida) {

	int size= strlen(data);
	int in[2], out[2], stdout_size;
	//Crea 2 pipes, uno para stdin y otro para stdout. in[0] y out[0] se usan para leer e in[1] y out[1] para escribir
	if (pipe(in) < 0) {
		error("pipe in");
	}

	if (pipe(out) < 0) {
		error("pipe out");
	}

	if (!fork()) {
		//Fork devuelve 0 en el hijo y el pid del hijo en el padre, así que acá estoy en el hijo.

		//Cierra stdin stdout y stdeer
		close(0);
		close(1);
		close(2);

		//Duplica stdin al lado de lectura in y stdout y stdeer al lado de escritura out
		dup2(in[0], 0);
		dup2(out[1], 1);
		dup2(out[1], 2);

		//Cierra los lados del pipe que se usan en el padre
		close(in[1]);
		close(out[0]);

		//La imagen del proceso hijo se reemplaza con la del path_ejecutable
		execl(path_ejecutable, path_ejecutable ,NULL);
		//On success acá nunca llega porque la imágen (incluido el código) se reemplazó en la lina anterior
		error("No se pudo ejecutar el proceso");
	}

	//Cierra los lados de los pipes que se usan en el hijo
	close(in[0]);
	close(out[1]);

	//Escribe en el lado de escritura del pipe que el proceso hijo va a ver como stdin
	write(in[1], data, size);
	//Se cierra para generar un EOF y que el proceso hijo termine de leer de stdin
	close(in[1]);

	FILE* temp_file = fopen(path_salida, "w");

	//Lee del lado de lectura del pipe que el proceso hijo tiene como stdout y lo paso al buffer
	//Hay que ver el tema del tamaño del buffer
	int buffer_size = BLOCK_SIZE_20MB * 2;
	char* buf = (char*)malloc(buffer_size);
	stdout_size = read(out[0], buf, buffer_size);
	//Agrega caracter nulo al final del stream del buffer
	buf[stdout_size] = 0;
	//Escribe el contenido del buffer en el archivo de salida
	fwrite(buf, stdout_size, 1, temp_file);
	fclose(temp_file);
	free(buf);
}
