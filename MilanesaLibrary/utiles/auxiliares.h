#ifndef UTILES_H_
#define UTILES_H_

#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <commons/string.h>

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

/****************** FUNCIONES AUXILIARES. ******************/

/*
 * Genera una nueva secuencia de enteros pseudo-random a retornar por rand().
 */
void seedgen(void);

void free_null(void** data);

void free_puntero_puntero(char** lines);

#endif /* UTILES_H_ */
