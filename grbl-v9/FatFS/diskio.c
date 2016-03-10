/**
 * \file
 *
 * \brief Implementation of low level disk I/O module skeleton for FatFS.
 *
 * Copyright (c) 2012 - 2014 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
/// @cond 0
/**INDENT-OFF**/
 /**
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/// @endcond

#include "diskio.h"
#include "sam.h"
  
  
//#include "ctrl_access.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

/**
 * \defgroup thirdparty_fatfs_port_group Port of low level driver for FatFS
 *
 * Low level driver for FatFS. The driver is based on the ctrl access module
 * of the specific MCU device.
 *
 * @{
 */
#define PASS      0
#define FAIL      1
#define LOW       0
#define HIGH      1

#define spi_cs_low() while(!(PIOD->PIO_PDSR & (PIO_PD8))); do { PIOA->PIO_CODR = PIO_PA28; } while (0)
#define spi_cs_high() do { PIOA->PIO_SODR = PIO_PA28; } while (0)
//#define spi_cs_low() do ; while (0)
//#define spi_cs_high() do ; while (0)


struct hwif {
	int initialized;
	int sectors;
	int erase_sectors;
	int capabilities;
};
typedef struct hwif hwif;

#define CAP_VER2_00	(1<<0)
#define CAP_SDHC	(1<<1)


enum sd_speed { SD_SPEED_INVALID, SD_SPEED_400KHZ, SD_SPEED_25MHZ };
typedef enum
{
	CTRL_GOOD       = PASS,     //!< Success, memory ready.
	CTRL_FAIL       = FAIL,     //!< An error occurred.
	CTRL_NO_PRESENT = FAIL + 1, //!< Memory unplugged.
	CTRL_BUSY       = FAIL + 2  //!< Memory not initialized or changed.
} Ctrl_status;
#define _BV(bit) (1 << (bit))

#ifdef __UINT32_TYPE__
typedef __UINT32_TYPE__ uint32_t;
#endif
#ifdef __UINT16_TYPE__
typedef __UINT16_TYPE__ uint16_t;
#endif
#ifdef __UINT8_TYPE__
typedef __UINT8_TYPE__ uint8_t;
#endif
typedef uint8_t                 U8 ;  //!< 8-bit unsigned integer.
typedef uint32_t                 U32 ;
typedef uint16_t                 U16 ;

hwif hw1;


static U8 crc7_one(U8 t, U8 data);
static int sd_read_csd(hwif *hw);




/** Default sector size */
#define SECTOR_SIZE_DEFAULT 512

/** Supported sector size. These values are based on the LUN function:
 * mem_sector_size(). */
#define SECTOR_SIZE_512  1
#define SECTOR_SIZE_1024 2
#define SECTOR_SIZE_2048 4
#define SECTOR_SIZE_4096 8

/**
 * \brief Initialize a disk.
 *
 * \param drv Physical drive number (0..).
 *
 * \return 0 or disk status in combination of DSTATUS bits
 *         (STA_NOINIT, STA_PROTECT).
 */
static void spi_init(void)
{
	PMC->PMC_PCER0 |= _BV(ID_PIOA);	//Enable Clock PortA
	
	//configure for input			//MISO
	PIOA->PIO_PDR |= PIO_PA25;
	PIOA->PIO_ODR |= PIO_PA25;		//Input
	
	PIOA->PIO_PDR |= PIO_PA26;		//MOSI
	PIOA->PIO_OER |= PIO_PA26;		//MOSI	Output
	PIOA->PIO_ABSR &= ~PIO_PA26;	//Peripheral A
	
	PIOA->PIO_PDR |= PIO_PA27;		//SPCK
	PIOA->PIO_OER |= PIO_PA27;		//SPCK	Output
	PIOA->PIO_ABSR &= ~PIO_PA27;	//Peripheral A
	
	PIOA->PIO_OER |= PIO_PA28;		//NPCS0	Output
/*	PIOA->PIO_ABSR &= ~PIO_PA28;	//Peripheral A
	PIOA->PIO_PUER |= PIO_PA28;		//pull-up*/

	//Enable clock for the SPI0 peripheral
	PMC->PMC_PCER0 |= _BV(ID_SPI0);

	//Disable the SPI0 peripheral so we can configure it.
	SPI0->SPI_CR = SPI_CR_SPIDIS;

	//Set as Master, Fixed Peripheral Select, Mode Fault Detection disabled and
	//	Peripheral Chip Select is PCS = xxx0 NPCS[3:0] = 1110
	SPI0->SPI_MR = SPI_MR_MSTR | SPI_MR_MODFDIS | 0x000e0000;
	
	//SPCK baudrate = MCK / SCBR = 84MHz / 128 = 656250Hz
	SPI0->SPI_CSR[0] |= 0x0000FF00  | 1 << 1;
	
	//Enable the SPI0 unit
	SPI0->SPI_CR = SPI_CR_SPIEN;


//	spi_set_speed(SD_SPEED_400KHZ);
}
U8 SPIWrite(uint8_t val)
{
	//Wait for previous transfer to complete
	while ((SPI0->SPI_SR & SPI_SR_TXEMPTY) == 0);
	
	//load the Transmit Data Register with the value to transmit
	SPI0->SPI_TDR = val;
	
	//Wait for data to be transferred to serializer
	while ((SPI0->SPI_SR & SPI_SR_TDRE) == 0);
    
	while ( (SPI0->SPI_SR & SPI_SR_RDRF) == 0 ) ;

    return SPI0->SPI_RDR & 0x00FF ;	
}

static void sd_cmd(U8 cmd, U32 arg)
{
	U8 crc = 0;
	SPIWrite(0x40 | cmd);
	crc = crc7_one(crc, 0x40 | cmd);
	SPIWrite(arg >> 24);
	crc = crc7_one(crc, arg >> 24);
	SPIWrite(arg >> 16);
	crc = crc7_one(crc, arg >> 16);
	SPIWrite(arg >> 8);
	crc = crc7_one(crc, arg >> 8);
	SPIWrite(arg);
	crc = crc7_one(crc, arg);
	//SPIWrite(0x95);	/* crc7, for cmd0 */
	SPIWrite(crc | 0x1);	/* crc7, for cmd0 */
}
static U8 crc7_one(U8 t, U8 data)
{
	int i;
	const U8 g = 0x89;

	t ^= data;
	for (i=0; i<8; i++) {
		if (t & 0x80)
		t ^= g;
		t <<= 1;
	}
	return t;
}

U8 crc7(const U8 *p, int len)
{
	int j;
	U8 crc = 0;
	for (j=0; j<len; j++)
	crc = crc7_one(crc, p[j]);

	return crc>>1;
}


static U8 sd_get_r1()
{
	int tries = 1000;
	U8 r;

	while (tries--) 
	{
		r = SPIWrite(0xff);
		if ((r & 0x80) == 0)
			return r;
	}
	return 0xff;
}

static U16 sd_get_r2()
{
	int tries = 1000;
	U16 r;

	while (tries--) {
		r = SPIWrite(0xff);
		if ((r & 0x80) == 0)
			break;
	}
	if (tries < 0)
		return 0xff;
	r = r<<8 | SPIWrite(0xff);

	return r;
}

/*
 * r1, then 32-bit reply... same format as r3
 */
static U8 sd_get_r7(U32 *r7)
{
	U32 r;
	r = sd_get_r1();
	if (r != 0x01)
		return r;

	r = SPIWrite(0xff) << 24;
	r |= SPIWrite(0xff) << 16;
	r |= SPIWrite(0xff) << 8;
	r |= SPIWrite(0xff);

	*r7 = r;
	return 0x01;
}
#define sd_get_r3 sd_get_r7

/*static const char *r1_strings[7] = {
	"in idle",
	"erase reset",
	"illegal command",
	"communication crc error",
	"erase sequence error",
	"address error",
	"parameter error"
};*/

/*static void print_r1(u8 r)
{
	int i;
	printf("R1: %02x\n", r);
	for (i=0; i<7; i++)
		if (r & (1<<i))
			printf("  %s\n", r1_strings[i]);
}*/

/*static const char *r2_strings[15] = {
	"card is locked",
	"wp erase skip | lock/unlock cmd failed",
	"error",
	"CC error",
	"card ecc failed",
	"wp violation",
	"erase param",
	"out of range | csd overwrite",
	"in idle state",
	"erase reset",
	"illegal command",
	"com crc error",
	"erase sequence error",
	"address error",
	"parameter error",
};*/

/*static void print_r2(u16 r)
{
	int i;
	printf("R2: %04x\n", r);
	for (i=0; i<15; i++)
		if (r & (1<<i))
			printf("  %s\n", r2_strings[i]);
}*/

/* Nec (=Ncr? which is limited to [0,8]) dummy bytes before lowering CS,
 * as described in sandisk doc, 5.4. */
static void sd_nec()
{
	int i;
	for (i=0; i<8; i++)
		SPIWrite(0xff);
}

static int sd_init(hwif *hw)
{
	int i;
	int r;
	U32 r7;
	U32 r3;
	int tries;
	U32 hcs = 0;

	hw->capabilities = 0;

	/* start with 100-400 kHz clock */
//	spi_set_speed(SD_SPEED_400KHZ);
	spi_cs_high();
	/* 74+ clocks with CS high */
	for (i=0; i<10; i++)
		SPIWrite(0xFF);

	/* reset */
	spi_cs_low();
	sd_cmd(0, 0);
	r = sd_get_r1();
	sd_nec();
	spi_cs_high();
	if (r == 0xff)
		goto err_spi;
	if (r != 0x01) {
//		printf("fail\n");
//		print_r1(r);
		goto err;
	}
//	printf("success\n");


//	printf("cmd8 - voltage.. ");
	/* ask about voltage supply */
	spi_cs_low();
	sd_cmd(8, 0x1aa /* VHS = 1 */);
	r = sd_get_r7(&r7);
	sd_nec();
	spi_cs_high();
	hw->capabilities |= CAP_VER2_00;
	if (r == 0xff)
		goto err_spi;
	if (r == 0x01)
	{
//		printf("success, SD v2.x\n");
	}else if (r & 0x4) {
		hw->capabilities &= ~CAP_VER2_00;
//		printf("not implemented, SD v1.x\n");
	} else {
//		printf("fail\n");
//		print_r1(r);
		return -2;
	}


//	printf("cmd58 - ocr.. ");
	/* ask about voltage supply */
	spi_cs_low();
	sd_cmd(58, 0);
	r = sd_get_r3(&r3);
	sd_nec();
	spi_cs_high();
	if (r == 0xff)
		goto err_spi;
	if (r != 0x01 && !(r & 0x4)) { /* allow it to not be implemented - old cards */
//		printf("fail\n");
//		print_r1(r);
		return -2;
	}
	else {
		int i1;
		for (i1=4; i1<=23; i1++)
			if (r3 & 1<<i1)
				break;
//		printf("Vdd voltage window: %i.%i-", (12+i)/10, (12+i)%10);
		for (i1=23; i1>=4; i1--)
			if (r3 & 1<<i1)
				break;
		/* CCS shouldn't be valid here yet */
//		printf("%i.%iV, CCS:%li, power up status:%li\n",(13+i)/10, (13+i)%10,	r3>>30 & 1, r3>>31);
//		printf("success\n");
	}


//	printf("acmd41 - hcs.. ");
	tries = 1000;
	/* say we support SDHC */
	if (hw->capabilities & CAP_VER2_00)
		hcs = 1<<30;

	/* needs to be polled until in_idle_state becomes 0 */
	do {
		/* send we don't support SDHC */
		spi_cs_low();
		/* next cmd is ACMD */
		sd_cmd(55, 0);
		r = sd_get_r1();
		sd_nec();
		spi_cs_high();
		if (r == 0xff)
			goto err_spi;
		/* well... it's probably not idle here, but specs aren't clear */
		if (r & 0xfe) {
//			printf("fail\n");
//			print_r1(r);
			goto err;
		}

		spi_cs_low();
		sd_cmd(41, hcs);
		r = sd_get_r1();
		sd_nec();
		spi_cs_high();
		if (r == 0xff)
			goto err_spi;
		if (r & 0xfe) {
//			printf("fail\n");
//			print_r1(r);
			goto err;
		}
	} while (r != 0 && tries--);
	if (tries == -1) {
//		printf("timeouted\n");
		goto err;
	}
//	printf("success\n");

	/* Seems after this card is initialized which means bit 0 of R1
	 * will be cleared. Not too sure. */


	if (hw->capabilities & CAP_VER2_00) {
//		printf("cmd58 - ocr, 2nd time.. ");
		/* ask about voltage supply */
		spi_cs_low();
		sd_cmd(58, 0);
		r = sd_get_r3(&r3);
		sd_nec();
		spi_cs_high();
		if (r == 0xff)
			goto err_spi;
		if (r & 0xfe) {
//			printf("fail\n");
//			print_r1(r);
			return -2;
		}
		else {
#if 1
			int i1;
			for (i1=4; i1<=23; i1++)
				if (r3 & 1<<i1)
					break;
//			printf("Vdd voltage window: %i.%i-", (12+i)/10, (12+i)%10);
			for (i1=23; i1>=4; i1--)
				if (r3 & 1<<i1)
					break;
			/* CCS shouldn't be valid here yet */
//			printf("%i.%iV, CCS:%li, power up status:%li\n",					(13+i)/10, (13+i)%10,					r3>>30 & 1, r3>>31);
			// XXX power up status should be 1 here, since we're finished initializing, but it's not. WHY?
			// that means CCS is invalid, so we'll set CAP_SDHC later
#endif
			if (r3>>30 & 1) {
				hw->capabilities |= CAP_SDHC;
			}

//			printf("success\n");
		}
	}


	/* with SDHC block length is fixed to 1024 */
	if ((hw->capabilities & CAP_SDHC) == 0) {
//		printf("cmd16 - block length.. ");
		spi_cs_low();
		sd_cmd(16, 512);
		r = sd_get_r1();
		sd_nec();
		spi_cs_high();
		if (r == 0xff)
			goto err_spi;
		if (r & 0xfe) {
//			printf("fail\n");
//			print_r1(r);
			goto err;
		}
//		printf("success\n");
	}


//	printf("cmd59 - enable crc.. ");
	/* crc on */
	spi_cs_low();
	sd_cmd(59, 0);
	r = sd_get_r1();
	sd_nec();
	spi_cs_high();
	if (r == 0xff)
		goto err_spi;
	if (r & 0xfe) {
//		printf("fail\n");
//		print_r1(r);
		goto err;
	}
//	printf("success\n");


	/* now we can up the clock to <= 25 MHz */
//	spi_set_speed(SD_SPEED_25MHZ);

	return 0;

 err_spi:
//	printf("fail spi\n");
	return -1;
 err:
	return -2;
}

static int sd_read_status(hwif *hw)
{
	U16 r2;

	spi_cs_low();
	sd_cmd(13, 0);
	r2 = sd_get_r2();
	sd_nec();
	spi_cs_high();
	if (r2 & 0x8000)
		return -1;
	//	if (r2)
	//		print_r2(r2);

	return 0;
}

static U16 crc16_ccitt(U16 crc, U8 ser_data)
{
	crc  = (U8)(crc >> 8) | (crc << 8);
	crc ^= ser_data;
	crc ^= (U8)(crc & 0xff) >> 4;
	crc ^= (crc << 8) << 4;
	crc ^= ((crc & 0xff) << 4) << 1;

	return crc;
}
U16 crc16(const U8 *p, int len)
{
	int i;
	U16 crc = 0;

	for (i=0; i<len; i++)
	crc = crc16_ccitt(crc, p[i]);

	return crc;
}

static int sd_get_data(hwif *hw, U8 *buf, int len)
{
	int tries = 20000;
	U8 r;
	U16 _crc16;
	U16 calc_crc;
	int i;

	while (tries--) {
		r = SPIWrite(0xff);
		if (r == 0xfe)
		break;
	}
	if (tries < 0)
	return -1;

	for (i=0; i<len; i++)
	buf[i] = SPIWrite(0xff);

	_crc16 = SPIWrite(0xff) << 8;
	_crc16 |= SPIWrite(0xff);

	calc_crc = crc16(buf, len);
	if (_crc16 != calc_crc) {
		//		printf("%s, crcs differ: %04x vs. %04x, len:%i\n", __func__, _crc16, calc_crc, len);
		return -1;
	}

	return 0;
}


static int sd_read_cid(hwif *hw)
{
	U8 buf[16];
	int r;

	spi_cs_low();
	sd_cmd(10, 0);
	r = sd_get_r1();
	if (r == 0xff) {
		spi_cs_high();
		return -1;
	}
	if (r & 0xfe) {
		spi_cs_high();
//		printf("%s ", __func__);
//		print_r1(r);
		return -2;
	}

	r = sd_get_data(hw, buf, 16);
	sd_nec();
	spi_cs_high();
	if (r == -1) {
//		printf("failed to get cid\n");
		return -3;
	}

/*	printf("CID: mid:%x, oid:%c%c, pnm:%c%c%c%c%c, prv:%i.%i, psn:%02x%02x%02x%02x, mdt:%i/%i\n",
			buf[0], buf[1], buf[2],			// mid, oid 
			buf[3], buf[4], buf[5], buf[6], buf[7],	// pnm 
			buf[8] >> 4, buf[8] & 0xf,		// prv
			buf[9], buf[10], buf[11], buf[12],	// psn
			2000 + (buf[13]<<4 | buf[14]>>4), 1 + (buf[14] & 0xf));*/

	return 0;
}

int hwif_init(hwif* hw)
{
	int tries = 2;
	//	Toto som zrusil ja aby sa vzdy inicializoval ked som to chcel
	//	if (hw->initialized)
	//		return 0;
	spi_init();
	while (tries--) {
		if (sd_init(hw) == 0)
		break;
	}
	if (tries == -1)
		return -1;

	/* read status register */
	sd_read_status(hw);
	sd_read_cid(hw);
	if (sd_read_csd(hw) != 0)
	return -1;
	hw->initialized = 1;
	return 0;
}

static int sd_read_csd(hwif *hw)
{
	U8 buf[16];
	int r;
	int capacity;

	spi_cs_low();
	sd_cmd(9, 0);
	r = sd_get_r1();
	if (r == 0xff) {
		spi_cs_high();
		return -1;
	}
	if (r & 0xfe) {
		spi_cs_high();
//		printf("%s ", __func__);
//		print_r1(r);
		return -2;
	}

	r = sd_get_data(hw, buf, 16);
	sd_nec();
	spi_cs_high();
	if (r == -1) {
//		printf("failed to get csd\n");
		return -3;
	}

	if ((buf[0] >> 6) + 1 == 1) {
	/* CSD v1 */
/*	printf("CSD: CSD v%i taac:%02x, nsac:%i, tran:%02x, classes:%02x%x, read_bl_len:%i, "		"read_bl_part:%i, write_blk_misalign:%i, read_blk_misalign:%i, dsr_imp:%i, "		"c_size:%i, vdd_rmin:%i, vdd_rmax:%i, vdd_wmin:%i, vdd_wmax:%i, "		"c_size_mult:%i, erase_blk_en:%i, erase_s_size:%i, "		"wp_grp_size:%i, wp_grp_enable:%i, r2w_factor:%i, write_bl_len:%i, write_bl_part:%i, "
		"filef_gpr:%i, copy:%i, perm_wr_prot:%i, tmp_wr_prot:%i, filef:%i\n",
			(buf[0] >> 6) + 1,
			buf[1], buf[2], buf[3],
			buf[4], buf[5] >> 4, 1<<(buf[5] & 0xf), //
			buf[6]>>7, (buf[6]>>6)&1, (buf[6]>>5)&1, (buf[6]>>4)&1,
			(buf[6]&0x3)<<10 | buf[7]<<2 | buf[8]>>6, // c_size 
			(buf[8]&0x38)>>3, buf[8]&0x07, buf[9]>>5, (buf[9]>>2)&0x7,
			1<<(2+(((buf[9]&3) << 1) | buf[10]>>7)), // c_size_mult 
			(buf[10]>>6)&1,
			((buf[10]&0x3f)<<1 | buf[11]>>7) + 1, // erase sector size 
			(buf[11]&0x7f) + 1, // write protect group size 
			buf[12]>>7, 1<<((buf[12]>>2)&7),
			1<<((buf[12]&3)<<2 | buf[13]>>6), // write_bl_len 
			(buf[13]>>5)&1,
			buf[14]>>7, (buf[14]>>6)&1, (buf[14]>>5)&1, (buf[14]>>4)&1,
			(buf[14]>>2)&3 // file format );*/

	capacity = (((buf[6]&0x3)<<10 | buf[7]<<2 | buf[8]>>6)+1) << (2+(((buf[9]&3) << 1) | buf[10]>>7)) << ((buf[5] & 0xf) - 9);
	/* ^ = (c_size+1) * 2**(c_size_mult+2) * 2**(read_bl_len-9) */

	} else {
	/* CSD v2 */
		/* this means the card is HC */
		hw->capabilities |= CAP_SDHC;

/*	printf("CSD: CSD v%i taac:%02x, nsac:%i, tran:%02x, classes:%02x%x, read_bl_len:%i, "
		"read_bl_part:%i, write_blk_misalign:%i, read_blk_misalign:%i, dsr_imp:%i, "
		"c_size:%i, erase_blk_en:%i, erase_s_size:%i, "
		"wp_grp_size:%i, wp_grp_enable:%i, r2w_factor:%i, write_bl_len:%i, write_bl_part:%i, "
		"filef_gpr:%i, copy:%i, perm_wr_prot:%i, tmp_wr_prot:%i, filef:%i\n",
			(buf[0] >> 6) + 1,
			buf[1], buf[2], buf[3],
			buf[4], buf[5] >> 4, 1<<(buf[5] & 0xf), / classes, read_bl_len
			buf[6]>>7, (buf[6]>>6)&1, (buf[6]>>5)&1, (buf[6]>>4)&1,
			buf[7]<<16 | buf[8]<<8 | buf[9], // c_size
			(buf[10]>>6)&1,
			((buf[10]&0x3f)<<1 | buf[11]>>7) + 1, // erase sector size
			(buf[11]&0x7f) + 1, / write protect group size
			buf[12]>>7, 1<<((buf[12]>>2)&7),
			1<<((buf[12]&3)<<2 | buf[13]>>6), // write_bl_len
			(buf[13]>>5)&1,
			buf[14]>>7, (buf[14]>>6)&1, (buf[14]>>5)&1, (buf[14]>>4)&1,
			(buf[14]>>2)&3 // file format ); */

	capacity = buf[7]<<16 | buf[8]<<8 | buf[9]; /* in 512 kB */
	capacity *= 1024; /* in 512 B sectors */

	}

//	printf("capacity = %i kB\n", capacity/2);
	hw->sectors = capacity;

	/* if erase_blk_en = 0, then only this many sectors can be erased at once
	 * this is NOT yet tested */
	hw->erase_sectors = 1;
	if (((buf[10]>>6)&1) == 0)
		hw->erase_sectors = ((buf[10]&0x3f)<<1 | buf[11]>>7) + 1;

	return 0;
}



DSTATUS disk_initialize(BYTE drv)
{
	if (hwif_init(&hw1) == 0)
		return 0;
	return STA_NOINIT;
}

/**
 * \brief  Return disk status.
 *
 * \param drv Physical drive number (0..).
 *
 * \return 0 or disk status in combination of DSTATUS bits
 *         (STA_NOINIT, STA_NODISK, STA_PROTECT).
 */
DSTATUS disk_status(BYTE drv)
{
	if (hw1.initialized)
		return 0;
	return STA_NOINIT;
}

static int sd_readsector(hwif *hw, U32 address, U8 *buf)
{
	int r;
	spi_cs_low();
	if (hw->capabilities & CAP_SDHC)
	sd_cmd(17, address); /* read single block */
	else
	sd_cmd(17, address*512); /* read single block */

	r = sd_get_r1();
	if (r == 0xff) {
		spi_cs_high();
		r = -1;
		goto fail;
	}
	if (r & 0xfe) {
		spi_cs_high();
		//		printf("%s\n", __func__);
		//		print_r1(r);
		r = -2;
		goto fail;
	}

	r = sd_get_data(hw, buf, 512);
	sd_nec();
	spi_cs_high();
	if (r == -1) {
		r = -3;
		goto fail;
	}

	return 0;
	fail:
	//	printf("failed to read sector %li, err:%i\n", address, r);
	return r;
}


int sd_read(hwif* hw, U32 address, U8 *buf)
{
	int r;
	int tries = 10;

	r = sd_readsector(hw, address, buf);

	while (r < 0 && tries--) {
		if (sd_init(hw) != 0)
		continue;

		/* read status register */
		sd_read_status(hw);

		r = sd_readsector(hw, address, buf);
	}
	//	if (tries == -1)
	//		printf("%s: couldn't read sector %li\n", __func__, address);

	return r;
}


/**
 * \brief  Read sector(s).
 *
 * \param drv Physical drive number (0..).
 * \param buff Data buffer to store read data.
 * \param sector Sector address (LBA).
 * \param count Number of sectors to read (1..255).
 *
 * \return RES_OK for success, otherwise DRESULT error code.
 */


DRESULT disk_read(BYTE drv, BYTE *buff, DWORD sector, BYTE count)
{
	int i;

	for (i=0; i<count; i++)
	if (sd_read(&hw1, sector+i, buff+512*i) != 0)
		return RES_ERROR;
	return RES_OK;
}

/**
 * \brief  Write sector(s).
 *
 * The FatFs module will issue multiple sector transfer request (count > 1) to
 * the disk I/O layer. The disk function should process the multiple sector
 * transfer properly. Do not translate it into multiple sector transfers to the
 * media, or the data read/write performance may be drastically decreased.
 *
 * \param drv Physical drive number (0..).
 * \param buff Data buffer to store read data.
 * \param sector Sector address (LBA).
 * \param count Number of sectors to read (1..255).
 *
 * \return RES_OK for success, otherwise DRESULT error code.
 */
static int sd_put_data(hwif *hw, const U8 *buf, int len)
{
	U8 r;
	int tries = 10;
	U16 crc;

	SPIWrite(0xfe); /* data start */

	while (len--)
	SPIWrite(*buf++);

	crc = crc16(buf, len);
	/* crc16 */
	SPIWrite(crc>>8);
	SPIWrite(crc);

	/* normally just one dummy read in between... specs don't say how many */
	while (tries--) {
		r = SPIWrite(0xff);
		if (r != 0xff)
		break;
	}
	if (tries < 0)
	return -1;

	/* poll busy, about 300 reads for 256 MB card */
	tries = 100000;
	while (tries--) {
		if (SPIWrite(0xff) == 0xff)
		break;
	}
	if (tries < 0)
	return -2;

	/* data accepted, WIN */
	if ((r & 0x1f) == 0x05)
	return 0;

	return r;
}


static int sd_writesector(hwif *hw, U32 address, const U8 *buf)
{
	int r;

	spi_cs_low();
	if (hw->capabilities & CAP_SDHC)
		sd_cmd(24, address); /* write block */
	else
		sd_cmd(24, address*512); /* write block */

	r = sd_get_r1();
	if (r == 0xff) {
		spi_cs_high();
		r = -1;
		goto fail;
	}
	if (r & 0xfe) {
		spi_cs_high();
//		printf("%s\n", __func__);
//		print_r1(r);
		r = -2;
		goto fail;
	}

	SPIWrite(0xff); /* Nwr (>= 1) high bytes */
	r = sd_put_data(hw, buf, 512);
	sd_nec();
	spi_cs_high();
	if (r != 0) {
//		printf("sd_put_data returned: %i\n", r);
		r = -3;
		goto fail;
	}

	/* efsl code is weird shit, 0 is error in there?
	 * not that it's properly handled or anything,
	 * and the return type is char, fucking efsl */
	return 0;
 fail:
//	printf("failed to write sector %li, err:%i\n", address, r);
	return r;
}
int sd_write(hwif* hw, U32 address,const U8 *buf)
{
	int r;
	int tries = 10;

	r = sd_writesector(hw, address, buf);

	while (r < 0 && tries--) {
		if (sd_init(hw) != 0)
		continue;

		/* read status register */
		sd_read_status(hw);

		r = sd_writesector(hw, address, buf);
	}
	//	if (tries == -1)
	//		printf("%s: couldn't write sector %li\n", __func__, address);

	return r;
}




#if _READONLY == 0
DRESULT disk_write(BYTE drv, BYTE const *buff, DWORD sector, BYTE count)
{
	int i;

	for (i=0; i<count; i++)
	if (sd_write(&hw1, sector+i, buff+512*i) != 0)
	return RES_ERROR;

	return RES_OK;
}

#endif /* _READONLY */

/**
 * \brief  Miscellaneous functions, which support the following commands:
 *
 * CTRL_SYNC    Make sure that the disk drive has finished pending write
 * process. When the disk I/O module has a write back cache, flush the
 * dirty sector immediately.
 * In read-only configuration, this command is not needed.
 *
 * GET_SECTOR_COUNT    Return total sectors on the drive into the DWORD variable
 * pointed by buffer.
 * This command is used only in f_mkfs function.
 *
 * GET_BLOCK_SIZE    Return erase block size of the memory array in unit
 * of sector into the DWORD variable pointed by Buffer.
 * When the erase block size is unknown or magnetic disk device, return 1.
 * This command is used only in f_mkfs function.
 *
 * GET_SECTOR_SIZE    Return sector size of the memory array.
 *
 * \param drv Physical drive number (0..).
 * \param ctrl Control code.
 * \param buff Buffer to send/receive control data.
 *
 * \return RES_OK for success, otherwise DRESULT error code.
 */
DRESULT disk_ioctl(BYTE drv, BYTE ctrl, void *buff)
{
	switch (ctrl) {
		case CTRL_SYNC:
		return RES_OK;
		case GET_SECTOR_SIZE:
		*(WORD*)buff = 512;
		return RES_OK;
		case GET_SECTOR_COUNT:
		*(DWORD*)buff = hw1.sectors;
		return RES_OK;
		case GET_BLOCK_SIZE:
		*(DWORD*)buff = hw1.erase_sectors;
		return RES_OK;
	}
	return RES_PARERR;
}








//@}

/// @cond 0
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/// @endcond
