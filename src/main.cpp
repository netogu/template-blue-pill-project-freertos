/* Simple LED task demo:
 *
 * The LED on PC13 is toggled in task1.
 */

#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/spi.h>

#include <usbcdc.h>
#include "w25.h"
#include "magsensor.h"



class Counter {
	public:
		int value;
		void count(void){value++;}
};


static void gpio_setup(void);
static void spi_setup(void);


#define mainECHO_TASK_PRIORITY				( tskIDLE_PRIORITY + 1 )

void vApplicationStackOverflowHook(xTaskHandle *pxTask,signed portCHAR *pcTaskName);


extern void
vApplicationStackOverflowHook(xTaskHandle *pxTask,signed portCHAR *pcTaskName) {
	(void)pxTask;
	(void)pcTaskName;
	for(;;);
}

static void
task1(void *args) {

	(void)args;
	uint16_t w25_mfn = 0;
	Counter MyCounter;
	MagAngleSensor AngleSensor;
	MyCounter.value=0;

	for (;;) {
		gpio_toggle(GPIOC,GPIO13);
		MyCounter.count();
	
		w25_mfn = w25_manuf_device(SPI1);
		usb_printf("Class Counter %d - %X\n", MyCounter.value, w25_mfn);
	  vTaskDelay((300/portTICK_PERIOD_MS));
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

// End
