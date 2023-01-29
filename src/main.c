/* Simple LED task demo:
 *
 * The LED on PC13 is toggled in task1.
 */
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/spi.h>

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

static void gpio_setup(void);
static void spi_setup(void);

static uint8_t w25_read_sr1(uint32_t spi);
static void w25_wait(uint32_t spi);
static uint16_t w25_manuf_device(uint32_t spi);


#define mainECHO_TASK_PRIORITY				( tskIDLE_PRIORITY + 1 )

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,signed portCHAR *pcTaskName);

void
vApplicationStackOverflowHook(xTaskHandle *pxTask,signed portCHAR *pcTaskName) {
	(void)pxTask;
	(void)pcTaskName;
	for(;;);
}



static void
task1(void *args) {

	(void)args;
	uint32_t counter = 0;
	uint16_t w25_mfn = 0;

	for (;;) {
		gpio_toggle(GPIOC,GPIO13);
		counter++;
		w25_mfn = w25_manuf_device(SPI1);
		usb_printf("Hello World %d - %X\n", counter, w25_mfn);

	  vTaskDelay((100/portTICK_PERIOD_MS));
	}
}

int
main(void) {

	gpio_setup();
	spi_setup();

	usb_start(true, configMAX_PRIORITIES-1);
	xTaskCreate(task1,"LED",100,NULL,configMAX_PRIORITIES-1,NULL);
	vTaskStartScheduler();

	for (;;)
		;
	return 0;
}

static void 
gpio_setup(void) {

	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
	rcc_periph_clock_enable(RCC_GPIOC);
	gpio_set_mode(GPIOC,GPIO_MODE_OUTPUT_2_MHZ,GPIO_CNF_OUTPUT_PUSHPULL,GPIO13);
}

static void 
spi_setup(void) {

	rcc_periph_clock_enable(RCC_SPI1);
	gpio_set_mode(GPIOA,
			GPIO_MODE_OUTPUT_50_MHZ,
			GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
			GPIO4|GPIO5|GPIO7);
	gpio_set_mode(GPIOA,
				GPIO_MODE_INPUT,
				GPIO_CNF_INPUT_FLOAT,
				GPIO6);

	spi_reset(SPI1);
	spi_init_master(SPI1,
			SPI_CR1_BAUDRATE_FPCLK_DIV_256,
			SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
			SPI_CR1_CPHA_CLK_TRANSITION_1,
			SPI_CR1_DFF_8BIT,
			SPI_CR1_MSBFIRST);

	spi_disable_software_slave_management(SPI1);
	spi_enable_ss_output(SPI1);
}

static uint8_t
w25_read_sr1(uint32_t spi) {
	uint8_t sr1;

	spi_enable(spi);
	spi_xfer(spi,W25_CMD_READ_SR1);
	sr1 = spi_xfer(spi,DUMMY);
	spi_disable(spi);
	return sr1;
}

static void
w25_wait(uint32_t spi) {

	while ( w25_read_sr1(spi) & W25_SR1_BUSY )
		taskYIELD();
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
// End
