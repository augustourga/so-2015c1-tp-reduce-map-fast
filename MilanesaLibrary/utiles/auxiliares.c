#include "auxiliares.h"

void free_null(void** data) {
	free(*data);
	*data = NULL;
	data = NULL;
}

void seedgen(void) {
	long seed;
	time_t seconds;
	seed = abs(((time(&seconds) * 181) * ((getpid() - 83) * 359)) % 104729);
	srand(seed);
}

void free_puntero_puntero(char** lines) {
	string_iterate_lines(lines, (void*) free);
	free(lines);
}
