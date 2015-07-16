#include "log.h"

t_log* logger_consola;
t_log* logger_interno;

//pthread_mutex_t mutex_logger_consola = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t mutex_logger_interno = PTHREAD_MUTEX_INITIALIZER;

void log_crear(char* log_level, char* ruta_log, char* proceso) {
	logger_consola = log_create(ruta_log, proceso, true, log_level_from_string(log_level));
	logger_interno = log_create(ruta_log, proceso, false, log_level_from_string(log_level));
}

void log_debug_consola(char* mensaje_template, ...) {
	va_list arguments;
	va_start(arguments, mensaje_template);
	//pthread_mutex_lock(&mutex_logger_consola);
	char* mensaje = string_from_vformat(mensaje_template, arguments);
	log_debug(logger_consola, mensaje);
	//pthread_mutex_unlock(&mutex_logger_consola);
	va_end(arguments);
	free(mensaje);
}

void log_info_consola(char* mensaje_template, ...) {
	va_list arguments;
	va_start(arguments, mensaje_template);
	char* mensaje = string_from_vformat(mensaje_template, arguments);
	//pthread_mutex_lock(&mutex_logger_consola);
	log_info(logger_consola, mensaje);
	//pthread_mutex_unlock(&mutex_logger_consola);
	va_end(arguments);
	free(mensaje);
}

void log_error_consola(char* mensaje_template, ...) {
	va_list arguments;
	va_start(arguments, mensaje_template);
	char* mensaje = string_from_vformat(mensaje_template, arguments);
	//pthread_mutex_lock(&mutex_logger_consola);
	log_error(logger_consola, mensaje);
	//pthread_mutex_unlock(&mutex_logger_consola);
	va_end(arguments);
	free(mensaje);
}

void log_debug_interno(char* mensaje_template, ...) {
	va_list arguments;
	va_start(arguments, mensaje_template);
	char* mensaje = string_from_vformat(mensaje_template, arguments);
	//pthread_mutex_lock(&mutex_logger_interno);
	log_debug(logger_interno, mensaje);
	//pthread_mutex_unlock(&mutex_logger_interno);
	va_end(arguments);
	free(mensaje);
}

void log_info_interno(char* mensaje_template, ...) {
	va_list arguments;
	va_start(arguments, mensaje_template);
	char* mensaje = string_from_vformat(mensaje_template, arguments);
	//pthread_mutex_lock(&mutex_logger_interno);
	log_info(logger_interno, mensaje);
	//pthread_mutex_unlock(&mutex_logger_interno);
	va_end(arguments);
	free(mensaje);
}

void log_error_interno(char* mensaje_template, ...) {
	va_list arguments;
	va_start(arguments, mensaje_template);
	char* mensaje = string_from_vformat(mensaje_template, arguments);
	//pthread_mutex_lock(&mutex_logger_interno);
	log_error(logger_interno, mensaje);
	//pthread_mutex_unlock(&mutex_logger_interno);
	va_end(arguments);
	free(mensaje);
}
