#ifndef _LINKEDLIST_H
#define _LINKEDLIST_H

#include "stdbool.h"
#include "integer.h"
#include "stdint-gcc.h"


struct TString{
	unsigned char handle;
	struct TString *next;
	struct TString *prev;
	char *text;	//	Toto pole moze mat roznu dlzku
} ;

extern void ll_add(char *text);
extern void ll_add_ABC(char *text);
extern uint16_t ll_count(void);
extern char* ll_item(uint16_t index);
extern void ll_clear(void);


#endif