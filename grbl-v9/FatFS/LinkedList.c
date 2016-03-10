#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "LinkedList.h"

struct TString *llStart = NULL;
struct TString *llEnd = NULL;
uint16_t llCount = 0;

void ll_add(char *text)	//	Pridam na koniec
{
	struct TString *novy,*aktualny;
	if(llCount > 100) return;
	novy = (struct TString*)malloc(sizeof(struct TString));
	novy->text = (char*)malloc(strlen(text) + 1);
	if(llStart == NULL)	//	Prazdny zaciatok, takze aj koniec
	{
		llStart = novy;
		llEnd = novy; 
		llEnd->next = NULL;
	} else // Nejaky ten zaznam uz existuje
	{
		aktualny = llStart;	//	Najdem zaciatok zoznamu
		while(true)
		{
			if(aktualny->next == llEnd)	//	Toto je posledny zaznam v zozname
			{
				aktualny->next = novy;
				break;				
			}
			aktualny = aktualny->next;
		}
	}
	memcpy(novy->text,text,strlen(text) + 1);
	novy->next = llEnd;
	llEnd->prev = novy;
	llCount++;
}
void ll_add_ABC(char *text)	//	Pridam podla abecedy
{
	struct TString *novy,*aktualny;
	novy = (struct TString*)malloc(sizeof(struct TString));
	novy->text = (char*)malloc(strlen(text) + 1);
	memcpy(novy->text,text,strlen(text) + 1);
//	debugprintf(300,"vkladam",text);
	llCount++;
	if(llStart == NULL)	//	Prazdny zaciatok, takze aj koniec
	{
		llStart = novy;
		llStart->prev = NULL;
		llEnd = novy;
		llEnd->next = NULL;
		llEnd->prev = novy;
		novy->next = NULL;
		novy->prev = NULL;
		return;
	} else // Nejaky ten zaznam uz existuje
	{
		aktualny = llStart;	//	Najdem zaciatok zoznamu
		while(true)
		{
//			debugprintf(300,"Nasiel som ",aktualny->text);
			if(strncmp(aktualny->text,text,strlen(text)) < 0)
			{	
				if(aktualny->next == NULL)	//	Toto je posledny zaznam v zozname, pridam teda novy zaznam na koniec
				{
					aktualny->next = novy;
					novy->prev = aktualny;
					novy->next = NULL;
					llEnd = novy;
					return;
				}
				aktualny = aktualny->next;
				continue;
			} else 
				break;				
			if(aktualny->next == NULL)	//	Toto je posledny zaznam v zozname, pridam teda novy zaznam na koniec
			{
				aktualny->next = novy;
				novy->prev = aktualny;
				novy->next = NULL;
				llEnd = novy;
				return;
			}
		}
	}
	if(aktualny == llStart)
		llStart = novy;
	novy->prev = aktualny->prev;
	(aktualny->prev)->next = novy;	//	Predosly bude ukazovat na novo vzniknuty
	aktualny->prev = novy;
	novy->next = aktualny;			//	Za novym bude aktualny
}

uint16_t ll_count(void)
{
	return llCount;		
}


char* ll_item(uint16_t index)
{
	struct TString *aktualny;
	if(index > (llCount - 1)) return NULL;
	aktualny = llStart;	//	Najdem zaciatok zoznamu
	while(true)
	{
		if(!index)
		{
			return(aktualny->text);
		}
		aktualny = aktualny->next;
		index--;
	}
}

void ll_clear(void)
{
	struct TString *aktualny = llStart;
	struct TString *next;
	if(!llStart) return;	//	Zoznam je prazdny

	while(aktualny->next)
	{
		next = aktualny->next;
		free(aktualny);
		aktualny = next;
	}
	if(aktualny)
		free(aktualny);
	llStart = NULL;
	llEnd = NULL;
	llCount = 0;	
}