#include "utiles.h"

int server_socket(uint16_t port)
{
	int sock_fd, optval = 1;
	struct sockaddr_in servername;

	/* Create the socket. */
	sock_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		perror("socket");
		return -1;
	}

	/* Set socket options. */
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) == -1) {
		perror("setsockopt");
		return -2;
	}

	/* Fill ip / port info. */
	servername.sin_family = AF_INET;
	servername.sin_addr.s_addr = htonl(INADDR_ANY);
	servername.sin_port = htons(port);

 	/* Give the socket a name. */
	if (bind(sock_fd, (struct sockaddr *) &servername, sizeof servername) < 0) {
		perror("bind");
		return -3;
	}

	/* Listen to incoming connections. */
	if (listen(sock_fd, 1) < 0) {
		perror("listen");
		return -4;
	}

	return sock_fd;
}


int client_socket(char *ip, uint16_t port)
{
	int sock_fd;
	struct sockaddr_in servername;

	/* Create the socket. */
	sock_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		perror("socket");
		return -1;
	}

	/* Fill server ip / port info. */
	servername.sin_family = AF_INET;
	servername.sin_addr.s_addr = inet_addr(ip);
	servername.sin_port = htons(port);
	memset(&(servername.sin_zero), 0, 8);

	/* Connect to the server. */
	if (connect(sock_fd, (struct sockaddr *) &servername, sizeof servername) < 0) {
		perror("connect");
		return -2;
	}

	return sock_fd;
}


int accept_connection(int sock_fd)
{
	struct sockaddr_in clientname;
	size_t size = sizeof clientname;

	int new_fd = accept(sock_fd, (struct sockaddr *) &clientname, (socklen_t *) &size);
	if (new_fd < 0) {
		perror("accept");
		return -1;
	}	

	return new_fd;
}


t_msg *id_message(t_msg_id id) {

	t_msg *new = malloc(sizeof *new);

	new->header.id = id;
	new->argv = NULL;
	new->stream = NULL;
	new->header.argc = 0;
	new->header.length = 0;
	
	return new;
}


t_msg *argv_message(t_msg_id id, uint16_t count, ...)
{
	va_list arguments;
	va_start(arguments, count);

	int32_t *val = malloc(count * sizeof *val);

	int i;
	for (i = 0; i < count; i++) {
		val[i] = va_arg(arguments, uint32_t);
	}

	t_msg *new = malloc(sizeof *new);
	new->header.id = id;
	new->header.argc = count;
	new->argv = val;
	new->header.length = 0;
	new->stream = NULL;

	va_end(arguments);

	return new;
}


t_msg *string_message(t_msg_id id, char *message, uint16_t count, ...)
{
	va_list arguments;
	va_start(arguments, count);

	int32_t *val = NULL;
	if(count > 0) 
		val = malloc(count * sizeof *val);

	int i;
	for (i = 0; i < count; i++) {
		val[i] = va_arg(arguments, uint32_t);
	}

	t_msg *new = malloc(sizeof *new);
	new->header.id = id;
	new->header.argc = count;
	new->argv = val;
	new->header.length = strlen(message);
	new->stream = string_duplicate(message);

	va_end(arguments);

	return new;
}


t_msg *modify_message(t_msg_id new_id, t_msg *old_msg, uint16_t new_count, ...)
{
	va_list arguments;
	va_start(arguments, new_count);

	uint16_t old_count = old_msg->header.argc;

	int32_t *val = malloc((new_count + old_count) * sizeof *val);

	int i;
	for (i = 0; i < new_count; i++) {
		val[i] = va_arg(arguments, uint32_t);
	}

	memcpy(val + new_count, old_msg->argv, old_count * sizeof(uint32_t));


	char *buffer = NULL;
	if(old_msg->header.length > 0)
		buffer = malloc(old_msg->header.length);

	memcpy(buffer, old_msg->stream, old_msg->header.length);

	t_msg *new = malloc(sizeof *new);
	new->header.id = new_id == NO_NEW_ID ? old_msg->header.id : new_id;
	new->header.argc = new_count + old_count;
	new->argv = val;
	new->header.length = old_msg->header.length;
	new->stream = buffer;

	va_end(arguments);

	destroy_message(old_msg);

	return new;
}


t_msg *remake_message(t_msg_id new_id, t_msg *old_msg, uint16_t new_count, ...)
{
	va_list arguments;
	va_start(arguments, new_count);

	int32_t *val = NULL;
	if(new_count > 0)
		val = malloc(new_count * sizeof *val);

	int i;
	for (i = 0; i < new_count; i++) {
		val[i] = va_arg(arguments, uint32_t);
	}

	char *buffer = NULL;	
	if(old_msg->header.length > 0) {
		buffer = malloc(old_msg->header.length);
		memcpy(buffer, old_msg->stream, old_msg->header.length);
	}


	t_msg *new = malloc(sizeof *new);
	new->header.id = new_id == NO_NEW_ID ? old_msg->header.id : new_id;
	new->header.argc = new_count;
	new->argv = val;
	new->header.length = old_msg->header.length;
	new->stream = buffer;

	va_end(arguments);

	destroy_message(old_msg);

	return new;
}


t_msg *beso_message(t_msg_id id, char *beso_path, uint16_t count, ...)
{
	va_list arguments;
	va_start(arguments, count);

	int32_t *val = NULL;
	if(count > 0)
		val = malloc(count * sizeof *val);

	int i;
	for (i = 0; i < count; i++) {
		val[i] = va_arg(arguments, uint32_t);
	}

	FILE *f = fopen(beso_path, "rb");
	if (f == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}
	
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buffer = malloc(fsize);

	fread(buffer, fsize, 1, f);

	t_msg *new = malloc(sizeof *new);
	new->header.id = id;
	new->header.argc = count;
	new->argv = val;
	new->header.length = fsize;
	new->stream = buffer;

	va_end(arguments);

	fclose(f);

	return new;
}
//
//
//t_msg *tcb_message(t_msg_id id, t_hilo *tcb, uint16_t count, ...)
//{
//	va_list arguments;
//	va_start(arguments, count);
//
//	int32_t *val = malloc(count * sizeof *val);
//
//	int i;
//	for (i = 0; i < count; i++) {
//		val[i] = va_arg(arguments, uint32_t);
//	}
//
//	size_t size = sizeof *tcb;
//	char *buffer = malloc(size);
//
//	memcpy(buffer, tcb, size);
//
//	t_msg *new = malloc(sizeof *new);
//	new->header.id = id;
//	new->header.argc = count;
//	new->argv = val;
//	new->header.length = size;
//	new->stream = buffer;
//
//	va_end(arguments);
//
//	return new;
//}


t_msg *recibir_mensaje(int sock_fd)
{
	t_msg *msg = malloc(sizeof *msg);
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


int enviar_mensaje(int sock_fd, t_msg *msg)
{
	int total = 0;
	int pending = msg->header.length + sizeof(t_header) + msg->header.argc * sizeof(uint32_t);
	char *buffer = malloc(pending);

 	/* Fill buffer with the struct's data. */
	memcpy(buffer, &(msg->header), sizeof(t_header));

	int i;
	for (i = 0; i < msg->header.argc; i++)
		memcpy(buffer + sizeof(t_header) + i * sizeof(uint32_t), msg->argv + i, sizeof(uint32_t));

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


void destroy_message(t_msg *msg)
{
	if(msg->header.length && msg->stream != NULL)
		free(msg->stream);
	if(msg->header.argc && msg->argv != NULL)
		free(msg->argv);
	free(msg);
}

void seedgen(void)
{
	long seed;
	time_t seconds;
	seed = abs(((time(&seconds) * 181) * ((getpid() - 83) * 359)) % 104729);
	srand(seed);
}


//t_hilo *retrieve_tcb(t_msg *msg)
//{
//	t_hilo *new = malloc(sizeof *new);
//	memcpy(new, msg->stream, sizeof *new);
//	return new;
//}


void create_file(char *path,size_t size) {

	FILE *f = fopen(path, "wb");

	fseek(f,size-1,SEEK_SET);

	fputc('\n', f);

	fclose(f);
}


void clean_file(char *path) {

	FILE *f = fopen(path, "wb");

	fclose(f);
}


char* read_file(char *path, size_t size) {

	FILE *f = fopen(path, "rb");
	if(f == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	char *buffer = malloc(size + 1);
	if(buffer == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	fread(buffer, size, 1, f);

	fclose(f);

	buffer[size] = '\0';

	return buffer;
}


void memcpy_from_file(char *dest, char *path, size_t size) {
	
	FILE *f = fopen(path, "rb");

	if(f != NULL) {
		fread(dest, size, 1, f);
		fclose(f);
	}
}


char *read_file_and_clean(char *path, size_t size) {

	FILE *f = fopen(path, "rb");
	if(f == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	char *buffer = malloc(size + 1);
	if(buffer == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	fread(buffer, size, 1, f);

	fclose(f);

	f = fopen(path, "wb");

	fclose(f);

	buffer[size] = '\0';

	return buffer;
}


char *read_whole_file(char *path) {

	FILE *f = fopen(path, "rb");
	if(f == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buffer = malloc(fsize + 1);
	if(buffer == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	fread(buffer, fsize, 1, f);

	fclose(f);

	buffer[fsize] = '\0';

	return buffer;
}


char *read_whole_file_and_clean(char *path) {

	FILE *f = fopen(path, "rb");
	if(f == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buffer = malloc(fsize + 1);
	if(buffer == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	fread(buffer, fsize, 1, f);

	fclose(f);

	buffer[fsize] = '\0';

	return buffer;
}


void write_file(char *path, char *data, size_t size) {

	FILE *f = fopen(path, "wb");

	fwrite(data, 1, size, f);

	fclose(f);
}


void print_msg(t_msg *msg)
{
	int i;
	puts("\n==================================================");
	printf("CONTENIDOS DEL MENSAJE:\n");
	printf("- ID: %s\n", id_string(msg->header.id));

	for (i = 0; i < msg->header.argc; i++) {;
		printf("- ARGUMENTO %d: %d\n", i + 1, msg->argv[i]);
	}

	printf("- TAMAÑO: %d\n", msg->header.length);
	printf("- CUERPO: ");

	for (i = 0; i < msg->header.length; i++)
		putchar(*(msg->stream + i));
	puts("\n==================================================\n");
}

char *id_string(t_msg_id id)
{
	char *buf;
	switch(id) {
		case NO_NEW_ID:
			buf = strdup("NO_NEW_ID");
			break;
		case EJECUTAR_MAP:
			buf = strdup("EJECUTAR_MAP");
			break;
		case EJECUTAR_REDUCE:
			buf = strdup("EJECUTAR_REDUCE");
			break;
		case FIN_MAP:
			buf = strdup("FIN_MAP");
			break;
		case FIN_REDUCE:
			buf = strdup("FIN_REDUCE");
			break;
		default:
			buf = string_from_format("%d, <AGREGAR A LA LISTA>", id);
			break;
	}

	return buf;
}


//void print_tcb(t_hilo *tcb)
//{
//	int i;
//	puts("CONTENIDOS DEL TCB");
//	printf("Registro PID Valor:		%8d\n", tcb->pid);
//	printf("Registro TID Valor:		%8d\n", tcb->tid);
//	printf("Registro KM Valor:		%8d\n", tcb->kernel_mode);
//	printf("Registro CS Valor:		%8d\n", tcb->segmento_codigo);
//	printf("Registro CS_Size Valor:		%8d\n", tcb->segmento_codigo_size);
//	printf("Registro IP Valor:		%8d\n", tcb->puntero_instruccion);
//	printf("Registro Stack Valor:		%8d\n", tcb->base_stack);
//	printf("Registro Stack_Size Valor:	%8d\n", tcb->cursor_stack);
//
//	for(i = 0;i < 5; ++i)
//		printf("Registro %c. Valor:		%8d\n",('A'+i), tcb->registros[i]);
//	printf("Registro COLA:			%8d\n", tcb->cola);
//	puts("\n");
//}


//char *string_cola(t_cola cola)
//{
//	switch(cola) {
//		case NEW:
//			return "NEW";
//		case READY:
//			return "READY";
//		case BLOCK:
//			return "BLOCK";
//		case EXEC:
//			return "EXEC";
//		case EXIT:
//			return "EXIT";
//		default:
//			return NULL;
//	}
//}
