#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>
#include "sockets.h"
#include "serializacion.h"
//

//
////antes de usar las funciones deserializar: fijarse que devuelve un registro con los parametros
//
//t_pedido_planificador* deserializar_pedido_planificador(t_bloque* bloque ){
//
//	t_pedido_planificador* pedido =malloc (sizeof(t_pedido_planificador));
//	pedido->tcb = malloc(sizeof(t_TCB));
//    t_bloque* bloque2;
//
//	int offset = 0, tmp_size = 0;
//
//	memcpy(&pedido->quantum, bloque->data + offset, tmp_size = sizeof(pedido->quantum));
//	offset += tmp_size;
//
//   bloque2= bloque_create(bloque->size- offset);
//   bloque2->data= bloque->data + offset ;
//   pedido->tcb= deserializar_tcb(bloque2);
//
//   bloque_destroy(bloque2);
//
//
//	 return pedido;
//}
//
//t_bloque*  serializar_mensaje_map(t_mensaje_map* mens){
//
//	int offset=0;
//
//	char code = '4';
//	t_bloque* bloque = bloque_create(3*sizeof(int)+mens.tamanio_ejecutable+sizeof(char*));
//
//	memcpy(bloque->data + offset, &code, sizeof(code));
//		                offset += sizeof(code);
//    memcpy(bloque->data + offset, &(mens->tamanio_ejecutable), sizeof(mens->tamanio_ejecutable));
//		                offset += sizeof(mens->tamanio_ejecutable);
//	memcpy(bloque->data + offset, &(mens->ejecutable->bitarray), sizeof(tam));
//			            offset += sizeof(tam);
//	return bloque;
//
//}
//
//t_bloque*  serializar_tabla_paginas(int pid){
//
//	int offset=0;
//
//	t_bloque* bloque = bloque_create(sizeof(int));
//
//	memcpy(bloque->data + offset, &pid, sizeof(pid));
//		                offset += sizeof(pid);
//return bloque;
//}
//
//int deserializar_tabla_paginas(t_bloque* bloque){
//        int ret;
//		int offset = 0, tmp_size = 0 ;
//
// memcpy(&ret, bloque->data + offset, tmp_size = sizeof(ret));
//     	    offset += tmp_size;
//
//return ret;
//
//}
//
//
//t_bloque*  serializar_leer_memoria(int pid,unsigned int dir,int tam){
//
//	int offset=0;
//
//	t_bloque* bloque = bloque_create(2*sizeof(int)+sizeof(dir));
//
//	memcpy(bloque->data + offset, &pid, sizeof(pid));
//		                offset += sizeof(pid);
//    memcpy(bloque->data + offset, &dir, sizeof(dir));
//		                offset += sizeof(dir);
//	memcpy(bloque->data + offset, &tam, sizeof(tam));
//			            offset += sizeof(tam);
//	return bloque;
//}
//
//t_leer_memoria* deserializar_leer_memoria(t_bloque* bloque){
//
//	t_leer_memoria* reg = malloc(sizeof(t_leer_memoria));
//			int offset = 0, tmp_size = 0 ;
//
// memcpy(&reg->pid, bloque->data + offset, tmp_size = sizeof(reg->pid));
//     	    offset += tmp_size;
//
// memcpy(&reg->dir, bloque->data + offset, tmp_size = sizeof(reg->dir));
//	    	offset += tmp_size;
//
// memcpy(&reg->tam, bloque->data + offset, tmp_size = sizeof(reg->tam));
//			offset += tmp_size;
//return reg;
//
//}
//
//
//t_bloque*  serializar_escribir_memoria(int pid,unsigned int dir,int tam, char* texto){
//
//	int offset=0;
//
//	t_bloque* bloque = bloque_create(2*sizeof(int)+sizeof(dir)+sizeof(texto));
//
//	memcpy(bloque->data + offset, &pid, sizeof(pid));
//		                offset += sizeof(pid);
//    memcpy(bloque->data + offset, &dir, sizeof(dir));
//		                offset += sizeof(dir);
//	memcpy(bloque->data + offset, &tam, sizeof(tam));
//			            offset += sizeof(tam);
//    memcpy(bloque->data + offset, &texto, sizeof(texto));
//         	            offset += sizeof(texto);
//	return bloque;
//}
//
//
//t_escribir_memoria* deserializar_escribir_memoria(t_bloque* bloque){
//
//	t_escribir_memoria* reg = malloc(sizeof(t_escribir_memoria));
//			int offset = 0, tmp_size = 0 ;
//
// memcpy(&reg->pid, bloque->data + offset, tmp_size = sizeof(reg->pid));
//     	    offset += tmp_size;
//
// memcpy(&reg->dir, bloque->data + offset, tmp_size = sizeof(reg->dir));
//	    	offset += tmp_size;
//
// memcpy(&reg->tam, bloque->data + offset, tmp_size = sizeof(reg->tam));
//			offset += tmp_size;
//
// memcpy(&reg->texto, bloque->data + offset, tmp_size = sizeof(reg->texto));
//			offset += tmp_size;
//
//return reg;
//
//}
//
//t_bloque*  serializar_crear_segmento(int pid, int tam){
//
//	int offset=0;
//
//	t_bloque* bloque = bloque_create(2*sizeof(int));
//
//	memcpy(bloque->data + offset, &pid, sizeof(pid));
//		                offset += sizeof(pid);
//	memcpy(bloque->data + offset, &tam, sizeof(tam));
//			            offset += sizeof(tam);
//	return bloque;
//}
//
//t_crear_segmento* deserializar_crear_segmento(t_bloque* bloque){
//
//	t_crear_segmento* reg = malloc(sizeof(t_crear_segmento));
//			int offset = 0, tmp_size = 0 ;
//
// memcpy(&reg->pid, bloque->data + offset, tmp_size = sizeof(reg->pid));
//     	    offset += tmp_size;
//
// memcpy(&reg->tam, bloque->data + offset, tmp_size = sizeof(reg->tam));
//			offset += tmp_size;
//return reg;
//
//}
//
//t_bloque*  serializar_tcb(t_TCB* tcb){
//
//	int offset=0;
//
//	t_bloque* bloque= bloque_create(4*sizeof(int)+ sizeof(tcb->km)+ 3*sizeof(unsigned int));
//
//	memcpy(bloque->data + offset, &tcb->km, sizeof(tcb->km));
//		                offset += sizeof(tcb->km);
//	memcpy(bloque->data + offset, &tcb->m, sizeof(tcb->m));
//			            offset += sizeof(tcb->m);
//	memcpy(bloque->data + offset, &tcb->p, sizeof(tcb->p));
//						offset += sizeof(tcb->p);
//	memcpy(bloque->data + offset, &tcb->x, sizeof(tcb->x));
//						offset += sizeof(tcb->x);
//	memcpy(bloque->data + offset, &tcb->s, sizeof(tcb->s));
//						offset += sizeof(tcb->s);
//	memcpy(bloque->data + offset, &tcb->tam, sizeof(tcb->tam));
//	                    offset += sizeof(tcb->tam);
//	memcpy(bloque->data + offset, &tcb->tid, sizeof(tcb->tid));
//		                offset += sizeof(tcb->tid);
//	memcpy(bloque->data + offset, &tcb->pid, sizeof(tcb->pid));
//			            offset += sizeof(tcb->pid);
//
//
//	t_bloque* bloquesito = serializar_reg(tcb->reg);
//	bloque->data = realloc(bloque->data, bloque->size + bloquesito->size);
//		memcpy(bloque->data + offset, bloquesito->data, bloquesito->size);
//		bloque->size += bloquesito->size;
//		offset += bloquesito->size;
//		bloque_destroy(bloquesito);
//
//
//return bloque;
//
//}
////t_reg_CPU* reg = malloc(sizeof(t_reg_CPU));
//
//
//
//
//t_bloque* serializar_reg(t_reg_CPU* reg){
//	int offset=0;
//
//		t_bloque* bloque= bloque_create(5*sizeof(int));
//
//		memcpy(bloque->data + offset, &reg->a, sizeof(reg->a));
//			                offset += sizeof(reg->a);
//		memcpy(bloque->data + offset, &reg->b, sizeof(reg->b));
//				            offset += sizeof(reg->b);
//		memcpy(bloque->data + offset, &reg->c, sizeof(reg->c));
//							offset += sizeof(reg->c);
//		memcpy(bloque->data + offset, &reg->d, sizeof(reg->d));
//							offset += sizeof(reg->d);
//		memcpy(bloque->data + offset, &reg->e, sizeof(reg->e));
//							offset += sizeof(reg->e);
//	return bloque;
//}
//
//t_TCB* deserializar_tcb(t_bloque* bloque){
//	t_TCB* 	tcb = malloc(sizeof(t_TCB));
//	tcb->reg = malloc(sizeof(t_reg_CPU));
//
//		int offset = 0, tmp_size = 0;
//
//		memcpy(&tcb->km, bloque->data + offset, tmp_size = sizeof(tcb->km));
//		offset += tmp_size;
//
//		memcpy(&tcb->m, bloque->data + offset, tmp_size = sizeof(tcb->m));
//		offset += tmp_size;
//
//		memcpy(&tcb->p, bloque->data + offset, tmp_size = sizeof(tcb->p));
//		offset += tmp_size;
//
//		memcpy(&tcb->x, bloque->data + offset, tmp_size = sizeof(tcb->x));
//		offset += tmp_size;
//
//		memcpy(&tcb->s, bloque->data + offset, tmp_size = sizeof(tcb->s));
//		offset += tmp_size;
//
//		memcpy(&tcb->tam, bloque->data + offset, tmp_size = sizeof(tcb->tam));
//		offset += tmp_size;
//
//		memcpy(&tcb->tid, bloque->data + offset, tmp_size = sizeof(tcb->tid));
//		offset += tmp_size;
//
//		memcpy(&tcb->pid, bloque->data + offset, tmp_size = sizeof(tcb->pid));
//		offset += tmp_size;
//
//         tcb->reg =  deserializar_reg( bloque->data + offset );
//
//
//		return tcb;
//	}
//
//t_reg_CPU* deserializar_reg(char* bloquesito){
//
//	t_reg_CPU* reg = malloc(sizeof(t_reg_CPU));
//			int offset = 0, tmp_size = 0 ;
//
// memcpy(&reg->a, bloquesito + offset, tmp_size = sizeof(reg->a));
//     	    offset += tmp_size;
//
// memcpy(&reg->b, bloquesito + offset, tmp_size = sizeof(reg->b));
//	    	offset += tmp_size;
//
// memcpy(&reg->c, bloquesito + offset, tmp_size = sizeof(reg->c));
//			offset += tmp_size;
//
// memcpy(&reg->d, bloquesito + offset, tmp_size = sizeof(reg->d));
//			offset += tmp_size;
//
// memcpy(&reg->e, bloquesito + offset, tmp_size = sizeof(reg->e));
//			offset += tmp_size;
//
//return reg;
//
//}
//
//

//
//void bloque_destroy(t_bloque* bloque) {
//	free(bloque->data);
//	free(bloque);
//}
//
//t_TCB* cargar_datos_tcb(){
//
//	t_TCB* tcb = malloc(sizeof(t_TCB));
//	tcb->reg =malloc(sizeof(t_reg_CPU));
//
//	tcb->km=0;
//	tcb->pid= 1;
//	tcb->tid= 1;
//	tcb->m=0;
//	tcb->p=0;
//	tcb->reg->a=0;
//	tcb->reg->b=0;
//	tcb->reg->c=0;
//	tcb->reg->d=0;
//	tcb->tam=0;
//	tcb->x=0;
//
//return tcb;
//}
//
//void imprimir_tcb(t_TCB* tcb){
//
//	printf("pid %d,tid %d,m %d, tam %d, a %d ", tcb->pid,tcb->tid,tcb->m, tcb->tam,tcb->reg->a);
//
//}
//#include "serializacion.h"
t_bloque* serializar_aceptacion_nodo(char* NODO_NUEVO,char* NOMBRE_NODO){

	int offset=0;
    int codigo=100;
    bool esNuevo;
		t_bloque* bloque = bloque_create(sizeof(int)+sizeof(bool)+sizeof(NOMBRE_NODO));

		  if(strncmp(NODO_NUEVO,"SI",2)==0)
		  { esNuevo= true;
		  }
		  else {esNuevo= false;
		  }

		memcpy(bloque->data + offset, &codigo, sizeof(codigo));
			                offset += sizeof(codigo);
		memcpy(bloque->data + offset, &esNuevo, sizeof(esNuevo));
			                offset += sizeof(esNuevo);
        memcpy(bloque->data + offset, &NOMBRE_NODO, sizeof(NOMBRE_NODO));
			                offset += sizeof(NOMBRE_NODO);
	return bloque;


}

t_aceptacion_nodo* deserializar_aceptacion_nodo(t_bloque* bloque) {
	t_aceptacion_nodo* msg = malloc(sizeof(t_aceptacion_nodo));
	int offset = 0, tmp_size = 0;
	tmp_size = sizeof(msg->nodo_nuevo);
	memcpy(&msg->nodo_nuevo, bloque->data, tmp_size);
	offset += tmp_size;
	tmp_size = sizeof(msg->nombre);
	memcpy(&msg->nombre, bloque->data + offset, tmp_size);

	printf("nombre %s,esNuevo %d",msg->nombre ,msg->nodo_nuevo);

	return msg;
}


t_bloque* bloque_create(int size) {
	t_bloque* bloque = malloc(sizeof(t_bloque));
	bloque->size = size;
	bloque->data = malloc(size);
	return bloque;
}

t_bitarray* serializar_mensaje_marta_job(t_Mensaje_marta_job* mensaje)
{
	int size= sizeof(char)+(int)mensaje->parametros->size;
	char* puntero= malloc(size);

	t_bitarray * bloque = bitarray_create(puntero,size);
	memcpy(&puntero[0], &mensaje->accion,1);
	memcpy(&puntero[1],mensaje->parametros->bitarray,mensaje->parametros->size);

	return bloque;
}

t_bitarray* serializar_marta_job_map(t_Marta_job_map* mensaje)
{
	int size = sizeof(2* sizeof(int)+ 16*sizeof(char));
	char* data = malloc(size);
	t_bitarray* bloque = bitarray_create(data,size);

	memcpy(data,&mensaje->bloque,sizeof(int));
	memcpy(data+sizeof(int),&mensaje->puerto_nodo,sizeof(int));
	memcpy(data-2*sizeof(int),&mensaje->ip_nodo,16*sizeof(char));

	return bloque;
}

t_Marta_job_map* deserializar_marta_job_map(t_bitarray* bloque)
{
	t_Marta_job_map* mensaje= malloc(sizeof(t_Mensaje_marta_job));

	memcpy(&mensaje->bloque, bloque->bitarray,sizeof(int));
	memcpy(&mensaje->puerto_nodo,bloque->bitarray+sizeof(int),sizeof(int));
	memcpy(mensaje->ip_nodo,bloque->bitarray+2*sizeof(int),16*sizeof(char));

	return mensaje;
}

t_Mensaje_marta_job* deserializar_mensaje_marta_job(t_bitarray* bloque)
{
	t_Mensaje_marta_job * mensaje = malloc(sizeof(t_Mensaje_marta_job));

	memcpy(&mensaje->accion,bloque,sizeof(char));
	memcpy(&mensaje->parametros,bloque+sizeof(char),bloque->size-sizeof(char));

	return mensaje;
}

t_bitarray* serializar_mensaje_job_marta(t_Mensaje_job_marta* mensaje)
{
	int size = sizeof(int)+ strlen(mensaje->archivos);
	char* data = malloc(size);
	t_bitarray* bloque = bitarray_create(data,size);
	bloque->size = size;
	bloque->bitarray =data;

	memcpy(data,&mensaje->codigo, sizeof(char));
	memcpy(data+sizeof(char),mensaje->archivos, size-sizeof(char));

	return bloque;
}

t_Mensaje_job_marta* deserializar_mensaje_job_marta(t_bitarray* bloque)
{
	t_Mensaje_job_marta* mensaje = malloc(sizeof(t_Mensaje_job_marta));
	memcpy(&mensaje->codigo,bloque->bitarray,sizeof(char));
	memcpy(mensaje->archivos,
		   bloque->bitarray+sizeof(char),
		   bloque->size-sizeof(char));

	return mensaje;
}

