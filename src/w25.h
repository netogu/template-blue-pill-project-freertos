#pragma once

#include "stdint.h"
#include <libopencm3/stm32/spi.h>


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <usbcdc.h>


#define W25_CMD_MANUF_DEVICE	0x90
#define W25_CMD_JEDEC_ID	0x9F
#define W25_CMD_WRITE_EN	0x06
#define W25_CMD_WRITE_DI	0x04
#define W25_CMD_READ_SR1	0x05
#define W25_CMD_READ_SR2	0x35
#define W25_CMD_CHIP_ERASE	0xC7
#define W25_CMD_READ_DATA	0x03
#define W25_CMD_FAST_READ	0x0B
#define W25_CMD_WRITE_DATA	0x02
#define W25_CMD_READ_UID	0x4B
#define W25_CMD_PWR_ON		0xAB
#define W25_CMD_PWR_OFF		0xB9
#define W25_CMD_ERA_SECTOR	0x20
#define W25_CMD_ERA_32K		0x52
#define W25_CMD_ERA_64K		0xD8

#define DUMMY			0x00

#define W25_SR1_BUSY		0x01
#define W25_SR1_WEL		0x02

static const char *cap[3] = {
	"W25X16",	// 14
	"W25X32",	// 15
	"W25X64"	// 16
};	

static uint8_t
w25_read_sr1(uint32_t spi) {
	uint8_t sr1;

	spi_enable(spi);
	spi_xfer(spi,W25_CMD_READ_SR1);
	sr1 = spi_xfer(spi,DUMMY);
	spi_disable(spi);
	return sr1;
}

static uint8_t
w25_read_sr2(uint32_t spi) {
	uint8_t sr1;

	spi_enable(spi);
	spi_xfer(spi,W25_CMD_READ_SR2);
	sr1 = spi_xfer(spi,DUMMY);
	spi_disable(spi);
	return sr1;
}

static void
w25_wait(uint32_t spi) {

	while ( w25_read_sr1(spi) & W25_SR1_BUSY )
		taskYIELD();
}

static bool
w25_is_wprotect(uint32_t spi) {

	w25_wait(spi);
	return !(w25_read_sr1(spi) & W25_SR1_WEL);
}

static void
w25_write_en(uint32_t spi,bool en) {

	w25_wait(spi);

	spi_enable(spi);
	spi_xfer(spi,en ? W25_CMD_WRITE_EN : W25_CMD_WRITE_DI);
	spi_disable(spi);

	w25_wait(spi);
}

static uint16_t
w25_manuf_device(uint32_t spi) {
	uint16_t info;

	w25_wait(spi);
	spi_enable(spi);
	spi_xfer(spi,W25_CMD_MANUF_DEVICE);	// Byte 1
	spi_xfer(spi,DUMMY);			// Dummy1 (2)
	spi_xfer(spi,DUMMY);			// Dummy2 (3)
	spi_xfer(spi,0x00);			// Byte 4
	info = spi_xfer(spi,DUMMY) << 8;	// Byte 5
	info |= spi_xfer(spi,DUMMY);		// Byte 6
	spi_disable(spi);
	return info;
}

static uint32_t
w25_JEDEC_ID(uint32_t spi) {
	uint32_t info;

	w25_wait(spi);
	spi_enable(spi);
	spi_xfer(spi,W25_CMD_JEDEC_ID);
	info = spi_xfer(spi,DUMMY);		 // Manuf.
	info = (info << 8) | spi_xfer(spi,DUMMY);// Memory Type
	info = (info << 8) | spi_xfer(spi,DUMMY);// Capacity
	spi_disable(spi);

	return info;
}

static void
w25_read_uid(uint32_t spi,void *buf,uint16_t bytes) {
	uint8_t *udata = (uint8_t*)buf;

	if ( bytes > 8 )
		bytes = 8;
	else if ( bytes <= 0 )
		return;

	w25_wait(spi);
	spi_enable(spi);
	spi_xfer(spi,W25_CMD_READ_UID);
	for ( uint8_t ux=0; ux<4; ++ux )
		spi_xfer(spi,DUMMY);
	for ( uint8_t ux=0; ux<bytes; ++ux )
		udata[ux] = spi_xfer(spi,DUMMY);
	spi_disable(spi);
}

static void
w25_power(uint32_t spi,bool on) {

	if ( !on )
		w25_wait(spi);
	spi_enable(spi);
	spi_xfer(spi,on ? W25_CMD_PWR_ON : W25_CMD_PWR_OFF);
	spi_disable(spi);
}

static bool
w25_chip_erase(uint32_t spi) {

	if ( w25_is_wprotect(spi) ) {
		usb_printf("Not Erased! Chip is not write enabled.\n");
		return false;
	}

	spi_enable(spi);
	spi_xfer(spi,W25_CMD_CHIP_ERASE);
	spi_disable(spi);

	usb_printf("Erasing chip..\n");

	if ( !w25_is_wprotect(spi) ) {
		usb_printf("Not Erased! Chip erase failed.\n");
		return false;
	}

	usb_printf("Chip erased!\n");
	return true;
}

static uint32_t		// New address is returned
w25_read_data(uint32_t spi,uint32_t addr,void *data,uint32_t bytes) {
	uint8_t *udata = (uint8_t*)data;

	w25_wait(spi);

	spi_enable(spi);
	spi_xfer(spi,W25_CMD_FAST_READ);
	spi_xfer(spi,addr >> 16);
	spi_xfer(spi,(addr >> 8) & 0xFF);
	spi_xfer(spi,addr & 0xFF);
	spi_xfer(spi,DUMMY);

	for ( ; bytes-- > 0; ++addr )
		*udata++ = spi_xfer(spi,DUMMY);

	spi_disable(spi);
	return addr;	
}


/* class W25Q32{ */

/* 	public: */

/* 		W25Q32(uint32_t spi_port) : spi(0){ */
/* 			spi = spi_port; */

/* 		} */

/* 		~W25Q32() { */
/* 		} */

/* 		bool is_busy() { */
/* 			uint8_t sr1; */

/* 			spi_enable(spi); */
/* 			spi_xfer(spi,W25_CMD_READ_SR1); */
/* 			sr1 = spi_xfer(spi,DUMMY); */
/* 			spi_disable(spi); */
/* 			usb_printf("SR1 = %X", sr1); */
/* 			return sr1 & W25_SR1_BUSY; */
/* 		} */

/* 		 uint16_t read_device_ids() { */
/* 			uint16_t info; */

/* 			wait_for_ready(); */
/* 			spi_enable(spi); */
/* 			spi_xfer(spi,W25_CMD_MANUF_DEVICE);	// Byte 1 */
/* 			spi_xfer(spi,DUMMY);			// Dummy1 (2) */
/* 			spi_xfer(spi,DUMMY);			// Dummy2 (3) */
/* 			spi_xfer(spi,0x00);			// Byte 4 */
/* 			info = spi_xfer(spi,DUMMY) << 8;	// Byte 5 */
/* 			info |= spi_xfer(spi,DUMMY);		// Byte 6 */
/* 			spi_disable(spi); */
/* 			return info; */
/* 		} */
	
/* 	private: */
/* 		uint32_t spi; */
		
/* 		inline void wait_for_ready(){ */

/* 			while (is_busy()){ */

/* 				taskYIELD(); */

/* 			} */
			
/* 		} */



/* }; */





