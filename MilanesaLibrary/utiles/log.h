/*
 * log.h
 *
 *  Created on: 5/6/2015
 *      Author: utnso
 */

#ifndef UTILES_LOG_H_
#define UTILES_LOG_H_

#include <commons/log.h>
#include <commons/string.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>


void log_crear(char* log_level, char* ruta_log, char* proceso);
void log_debug_consola(char* mensaje_template, ...);
void log_info_consola(char* mensaje_template, ...);
void log_error_consola(char* mensaje_template, ...);
void log_debug_interno(char* mensaje_template, ...);
void log_info_interno(char* mensaje_template, ...);
void log_error_interno(char* mensaje_template, ...);


#endif /* UTILES_LOG_H_ */
