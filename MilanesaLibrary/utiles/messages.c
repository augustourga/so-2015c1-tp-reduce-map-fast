#include "messages.h"

t_msg *id_message(t_msg_id id) {

	t_msg *new = malloc(sizeof *new);

	new->header.id = id;
	new->argv = NULL;
	new->stream = NULL;
	new->header.argc = 0;
	new->header.length = 0;

	return new;
}

t_msg *argv_message(t_msg_id id, uint16_t count, ...) {
	va_list arguments;
	va_start(arguments, count);

	int32_t *val = malloc(count * sizeof(int32_t));

	int i;
	for (i = 0; i < count; i++) {
		val[i] = va_arg(arguments, uint32_t);
	}

	t_msg *new = malloc(sizeof(t_msg));
	new->header.id = id;
	new->header.argc = count;
	new->argv = val;
	new->header.length = 0;
	new->stream = NULL;

	va_end(arguments);

	return new;
}

t_msg *string_message(t_msg_id id, char *message, uint16_t count, ...) {
	va_list arguments;
	va_start(arguments, count);

	int32_t *val = NULL;
	if (count > 0) {
		val = malloc(count * sizeof(int32_t));
	}

	int i;
	for (i = 0; i < count; i++) {
		val[i] = va_arg(arguments, uint32_t);
	}

	t_msg *new = malloc(sizeof(t_msg));
	new->header.id = id;
	new->header.argc = count;
	new->argv = val;
	new->header.length = strlen(message);
	new->stream = string_duplicate(message);

	va_end(arguments);

	return new;
}

//TODO: Borrar si no se usa
t_msg *modify_message(t_msg_id new_id, t_msg *old_msg, uint16_t new_count, ...) {
	va_list arguments;
	va_start(arguments, new_count);

	uint16_t old_count = old_msg->header.argc;

	int32_t *val = malloc((new_count + old_count) * sizeof(int32_t));

	int i;
	for (i = 0; i < new_count; i++) {
		val[i] = va_arg(arguments, uint32_t);
	}

	memcpy(val + new_count, old_msg->argv, old_count * sizeof(uint32_t));

	char *buffer = NULL;
	if (old_msg->header.length > 0) {
		buffer = malloc(old_msg->header.length);
	}

	memcpy(buffer, old_msg->stream, old_msg->header.length);

	t_msg *new = malloc(sizeof(t_msg));
	new->header.id = new_id == NO_NEW_ID ? old_msg->header.id : new_id;
	new->header.argc = new_count + old_count;
	new->argv = val;
	new->header.length = old_msg->header.length;
	new->stream = buffer;

	va_end(arguments);

	destroy_message(old_msg);

	return new;
}

t_msg *remake_message(t_msg_id new_id, t_msg *old_msg, uint16_t new_count, ...) {
	va_list arguments;
	va_start(arguments, new_count);

	int32_t *val = NULL;
	if (new_count > 0) {
		val = malloc(new_count * sizeof(int32_t));
	}

	int i;
	for (i = 0; i < new_count; i++) {
		val[i] = va_arg(arguments, uint32_t);
	}

	char *buffer = NULL;
	if (old_msg->header.length > 0) {
		buffer = malloc(old_msg->header.length);
		memcpy(buffer, old_msg->stream, old_msg->header.length);
	}

	t_msg *new = malloc(sizeof(t_msg));
	new->header.id = new_id == NO_NEW_ID ? old_msg->header.id : new_id;
	new->header.argc = new_count;
	new->argv = val;
	new->header.length = old_msg->header.length;
	new->stream = buffer;

	va_end(arguments);

	destroy_message(old_msg);

	return new;
}

t_msg *recibir_mensaje(int sock_fd) {
	t_msg *msg = malloc(sizeof(t_msg));
	msg->argv = NULL;
	msg->stream = NULL;

	/* Get message info. */
	int status = recv(sock_fd, &(msg->header), sizeof(t_header), MSG_WAITALL);
	if (status <= 0) {
		/* An error has ocurred or remote connection has been closed. */
		free(msg);
		return NULL;
	}

	/* Get message data. */
	if (msg->header.argc > 0) {
		msg->argv = malloc(msg->header.argc * sizeof(uint32_t));

		if (recv(sock_fd, msg->argv, msg->header.argc * sizeof(uint32_t), MSG_WAITALL) <= 0) {
			free(msg->argv);
			free(msg);
			return NULL;
		}
	}

	if (msg->header.length > 0) {
		msg->stream = malloc(msg->header.length + 1);

		if (recv(sock_fd, msg->stream, msg->header.length, MSG_WAITALL) <= 0) {
			free(msg->stream);
			free(msg->argv);
			free(msg);
			return NULL;
		}

		msg->stream[msg->header.length] = '\0';
	}

	return msg;
}

int enviar_mensaje(int sock_fd, t_msg *msg) {
	int total = 0;
	int pending = msg->header.length + sizeof(t_header) + msg->header.argc * sizeof(uint32_t);
	char *buffer = malloc(pending);

	/* Fill buffer with the struct's data. */
	memcpy(buffer, &(msg->header), sizeof(t_header));

	int i;
	for (i = 0; i < msg->header.argc; i++) {
		memcpy(buffer + sizeof(t_header) + i * sizeof(uint32_t), msg->argv + i, sizeof(uint32_t));
	}

	memcpy(buffer + sizeof(t_header) + msg->header.argc * sizeof(uint32_t), msg->stream, msg->header.length);

	/* Send message(s). */
	while (total < pending) {
		int sent = send(sock_fd, buffer, msg->header.length + sizeof msg->header + msg->header.argc * sizeof(uint32_t), MSG_NOSIGNAL);
		if (sent < 0) {
			free(buffer);
			return -1;
		}
		total += sent;
		pending -= sent;
	}

	free(buffer);

	return total;
}

void destroy_message(t_msg *msg) {
	if (msg->header.length && msg->stream != NULL) {
		free(msg->stream);
	}
	if (msg->header.argc && msg->argv != NULL) {
		free(msg->argv);
	}
	free(msg);
}

void print_msg(t_msg *msg) {
	int i;
	puts("\n==================================================");
	printf("CONTENIDOS DEL MENSAJE:\n");
	printf("- ID: %s\n", id_string(msg->header.id));

	for (i = 0; i < msg->header.argc; i++) {
		printf("- ARGUMENTO %d: %d\n", i + 1, msg->argv[i]);
	}

	printf("- TAMAÑO: %d\n", msg->header.length);
	printf("- CUERPO: ");

	for (i = 0; i < msg->header.length; i++) {
		putchar(*(msg->stream + i));
	}
	puts("\n==================================================\n");
}

char *id_string(t_msg_id id) {
	char *buf;
	switch (id) {
	case NO_NEW_ID:
		buf = strdup("NO_NEW_ID");
		break;
	case EJECUTAR_MAP:
		buf = strdup("EJECUTAR_MAP");
		break;
	case EJECUTAR_REDUCE:
		buf = strdup("EJECUTAR_REDUCE");
		break;
	case CONEXION_JOB:
		buf = strdup("CONEXION_JOB");
		break;
	case FIN_ENVIO_MENSAJE:
		buf = strdup("FIN_ENVIO_ARCH");
		break;
	case GET_BLOQUE:
		buf = strdup("GET_BLOQUE");
		break;
	case SET_BLOQUE:
		buf = strdup("SET_BLOQUE");
		break;
	case CONEXION_NODO:
		buf = strdup("INFO_NODO");
		break;
	case GET_FILE_CONTENT:
		buf = strdup("GET_FILE_CONTENT");
		break;
	default:
		buf = string_from_format("%d, <AGREGAR A LA LISTA>", id);
		break;
	}

	return buf;
}
