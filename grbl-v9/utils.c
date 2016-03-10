#include "sam.h"
#include "utils.h"
#include <stdlib.h>
#include "grbl.h"
#include "ra8875.h"
#include <stdio.h>
#include "FatFS/ff.h"
#include "FatFS/LinkedList.h"
volatile char realtime=0;

char buffer[128];
extern FATFS fs;

char filename[256];

OPTIMIZE_HIGH
//RAMFUNC
void portable_delay_cycles(unsigned long n)
{
	UNUSED(n);

	__asm (
		"loop: DMB	\n"
		"SUBS R0, R0, #1  \n"
		"BNE.N loop         "
	);
}



void delay_ms(int ms)
{
	while(ms--)
	{
		_delay_us(1000);
	}
}
void _delay_us(int us)
{
	volatile int x;
	while(us--)
	{
		x = 8;
		while(x--);
	}
}
void _delay_ms(int ms)
{
	delay_ms(ms);
}
uint32_t get_heap_free_size1( void )
{
	uint32_t high_mark= 600000;
	uint32_t low_mark = 0;
	uint32_t size ;
	void* p_mem;

	size = (high_mark + low_mark)/2;

	do
	{
		p_mem = malloc(size);
		if( p_mem != NULL)
		{ // Can allocate memory
			free(p_mem);
			low_mark = size;
		}
		else
		{ // Can not allocate memory
			high_mark = size;
		}

		size = (high_mark + low_mark)/2;
	}
	while( (high_mark-low_mark) >1 );

	return size;
}
void init_grbl(void)
{
    serial_reset_read_buffer(); // Clear serial read buffer
    gc_init(); // Set g-code parser to default state
    spindle_init();
    coolant_init();
    limits_init();
    probe_init();
    plan_reset(); // Clear block buffer and planner variables
    st_reset(); // Clear stepper subsystem variables.

    // Sync cleared gcode and planner positions to current system position.
    plan_sync_position();
    gc_sync_position();

    // Reset system variables.
    sys.abort = false;
    sys_rt_exec_state = 0;
    sys_rt_exec_alarm = 0;
    sys.suspend = false;
    // Start Grbl main loop. Processes program inputs and executes them.
}

void draw_pos()
{
	float x = sys.position[0] / DEFAULT_X_STEPS_PER_MM;
	float y = sys.position[1] / DEFAULT_Y_STEPS_PER_MM;
	float z = sys.position[2] / DEFAULT_Z_STEPS_PER_MM;
	sprintf(buffer," X=%10.3f Y=%10.3f Z=%10.3f",x,y,z);
	tft_textColor(RA8875_BLACK ,RA8875_LIME);
	tft_textWrite(1,1,0,0,buffer,ALINE_LEFT);
	tft_draw_press_unpress();
}


#define FILES_ON_SCREEN 10
FRESULT scan_files (char* path)
{
	FRESULT res;
	FILINFO fno;
	DIR dir;
	char *fn;   /* This function is assuming non-Unicode cfg. */
	#if _USE_LFN
	static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
	fno.lfname = lfn;
	fno.lfsize = sizeof lfn;
	#endif
	res = f_opendir(&dir, path);                       /* Open the directory */
	if (res == FR_OK) {
		for (;;) {
			res = f_readdir(&dir, &fno);                   /* Read a directory item */
			if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
			if (fno.fname[0] == '.') continue;             /* Ignore dot entry */
			#if _USE_LFN
			fn = *fno.lfname ? fno.lfname : fno.fname;
			#else
			fn = fno.fname;
			#endif
			if (fno.fattrib & AM_DIR)
			{                    /* It is a directory */
				char *temp;
				temp = (char*)malloc(strlen(path) + strlen(fn) + 3);
				sprintf(temp," %s",fn);
				ll_add_ABC(temp);
				free(temp);
			} else
			{                                       /* It is a file. */
				char *temp;
				temp = (char*)malloc(strlen(path) + strlen(fn) + 3);
				sprintf(temp,"%s",fn);
				ll_add_ABC(temp);
				free(temp);
			}
		}
	} else
	if(res == 13)
		tft_textWrite(10,100,780,0,(char*)"NO FILESYSTEM",ALINE_CENTER);
	return res;
}

FRESULT select_file( char*  path)
{
	FRESULT res;
	int a,FileOffset;
	f_mount(0,NULL);
	res = f_mount(0,&fs);
	if(res != FR_OK)
	return res;
	Action1 = 0;
	FileOffset = 0;
	tft_fillScreen(RA8875_BLACK);
	tft_delete_all_objects();
	tft_button(10,10,150,100,"Zruši",1,RA8875_LIME,10);
	ll_clear();
//	debugclear();
	if(strlen(path))
	ll_add_ABC((char*)" ..");
	scan_files(path);
	if(ll_count() > FILES_ON_SCREEN )
	tft_button(10,370,150,100,"~DOWN",2,RA8875_YELLOW,10);
	for(a = 0; a < FileOffset + FILES_ON_SCREEN; a++)
	{
		if(a >= ll_count()) break;
		if(!strncmp(ll_item(a)," ",1))	//	Adresar zacina medzerou
			tft_button(200,a * 48,590,36,ll_item(a),10 + a,RA8875_BLUE,BUTTONS_RADIUS);
		else
			tft_button(200,a * 48,590,36,ll_item(a),10 + a,RA8875_RED,BUTTONS_RADIUS);
	}
	while(1)
	{
		tft_draw_press_unpress();
		if(Action1 == 1)	//	CANCEL
		{
			Action1 = 0;
			break;
		}
		if(Action1 == 2)	//	NEXT
		{
			if(ll_count() > (FileOffset + FILES_ON_SCREEN))
			{
				FileOffset += FILES_ON_SCREEN;
				tft_delete_all_objects();
				tft_fillScreen(RA8875_BLACK);
				for(a = FileOffset; a < FileOffset + FILES_ON_SCREEN; a++)
				{
					if(a == ll_count() - 1) break;
					if(!strncmp(ll_item(a)," ",1))	//	Adresar zacina medzerou
						tft_button(200,(a * 48) % 480,590,36,ll_item(a),10 + a,RA8875_BLUE,BUTTONS_RADIUS);
					else
						tft_button(200,(a * 48) % 480,590,36,ll_item(a),10 + a,RA8875_RED,BUTTONS_RADIUS);
				}
				tft_button(10,10,150,100,"CANCEL",1,RA8875_LIME,BUTTONS_RADIUS);
				if(ll_count() > (FileOffset +FILES_ON_SCREEN))
				tft_button(10,370,150,100,"~DOWN",2,RA8875_YELLOW,BUTTONS_RADIUS);
				tft_button(10,220,150,100,"~UP",3,RA8875_YELLOW,BUTTONS_RADIUS);
			}
			else
			beep(600);
			Action1 = 0;
		}
		if(Action1 == 3)	//	Previous
		{
			if(FileOffset >= FILES_ON_SCREEN)
			{
				FileOffset -= FILES_ON_SCREEN;
				tft_delete_all_objects();
				tft_fillScreen(RA8875_BLACK);
				for(a = FileOffset; a < FileOffset + FILES_ON_SCREEN; a++)
				{
					if(a == ll_count() - 1) break;
					if(!strncmp(ll_item(a)," ",1))	//	Adresar zacina medzerou
						tft_button(200,(a * 48) % 480,590,36,ll_item(a),10 + a,RA8875_BLUE,BUTTONS_RADIUS);
					else
						tft_button(200,(a * 48) % 480,590,36,ll_item(a),10 + a,RA8875_RED,BUTTONS_RADIUS);
				}
				tft_button(10,10,150,100,"CANCEL",1,RA8875_LIME,BUTTONS_RADIUS);
				if(ll_count() > (FileOffset +FILES_ON_SCREEN))
				tft_button(10,370,150,100,"~DOWN",2,RA8875_YELLOW,BUTTONS_RADIUS);
				if(FileOffset)
					tft_button(10,220,150,100,"~UP",3,RA8875_YELLOW,BUTTONS_RADIUS);
			}
			else
			beep(600);
			Action1 = 0;
		}
		if(Action1 >= 10)
		{
			TGraphics *p;
			p = Gall;
			do
			{
				if(p->handle == Action1)
				{
					if(!strncmp(p->text," ",1))	//	je to adresar, nie subor lebo zacina medzerou
					{
						ll_clear();
						if(!strncmp(p->text," ..",3))	//	Nadradeny adresa minimalne druhej urovne
						{
							if(PosR('/',path) > 1)
							{
								char buffer[256];
								strcpy(buffer,path);
								buffer[PosR('/',path)] = 0;
								select_file((char*)buffer);
							}
							else
							{
								select_file((char*)"");
							}
						} else
						{
							char *fn = (char*)malloc(strlen(path) + strlen(p->text) + 3);
							sprintf(fn,"%s/%s",path,p->text + 1);
							select_file(fn);
							free(fn);
						}
					}else
					{
						sprintf(filename,"%s/%s",path,p->text);	//	Vybral som subor
						ll_clear();								//	zoznam suborov uz nepotrebujem
					}
					break;
				}
				p = p->next;
			}while(p);
			Action1 = 0;
			return FR_OK;
		}
	}
	return FR_NO_FILE;
}
