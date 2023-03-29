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
/* #include "w25.h" */
#include "magsensor.h"





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
SensorTask(void *args) {

	/* Magnetic Sensor : MA782
	 * MISO : PA6
	 * MOSI : PA7
	 * SCK : PA5
	 * NSS : PA4
	 * */

	(void)args;
	MagAngleSensor AngleSensor1;
	MagAngleSensor AngleSensor2;
	uint16_t angle1;
	uint16_t angle2;
	uint32_t counter = 0;
	AngleSensor1.begin(SPI1,GPIOA,GPIO3);
	AngleSensor2.begin(SPI1,GPIOA,GPIO4);

	for (;;) {
		gpio_toggle(GPIOC,GPIO13);
		angle1 = AngleSensor1.read_angle_raw();
		angle2 = AngleSensor2.read_angle_raw();
		usb_printf("%u\tAngle1=%u\tAngle2=%u\n", counter, angle1,angle2);
		counter++;
	  vTaskDelay((100/portTICK_PERIOD_MS));
	}
}

int main(void) {

	rcc_clock_setup_pll(&rcc_hse_configs[RCC_CLOCK_HSE8_72MHZ]);
	gpio_setup();
	spi_setup();

	usb_start(true, configMAX_PRIORITIES-1);
	xTaskCreate(SensorTask,"SensorTask",100,NULL,configMAX_PRIORITIES-1,NULL);
	vTaskStartScheduler();

	while(1){
		//Scheduler takes over before this
	}
	return 0;
}


static void 
gpio_setup(void) {

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOC);
	gpio_set_mode(GPIOC,GPIO_MODE_OUTPUT_2_MHZ,GPIO_CNF_OUTPUT_PUSHPULL,GPIO13);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ,GPIO_CNF_OUTPUT_PUSHPULL,GPIO3);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_10_MHZ,GPIO_CNF_OUTPUT_PUSHPULL,GPIO4);
	// Set SPI Device CS high
	gpio_set(GPIOA,GPIO3);
	gpio_set(GPIOA,GPIO4);

}

static void 
spi_setup(void) {

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_SPI1);

	// NSS=PA4, SCK=PA5, MOSI=PA7
	gpio_set_mode(GPIOA,
			GPIO_MODE_OUTPUT_50_MHZ,
			GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
			GPIO5|GPIO7);
	//MISO = PA6
	gpio_set_mode(GPIOA,
				GPIO_MODE_INPUT,
				GPIO_CNF_INPUT_FLOAT,
				GPIO6);

	spi_reset(SPI1);
	//FPCLK = 72Mhz
	//SCK = FPCLK / 64 = 1.125MHz
	//SCK = FPCLK / 8 = 9MHz
	//SCK = FPCLK / 4 = 18MHz
	spi_init_master(SPI1,
			SPI_CR1_BAUDRATE_FPCLK_DIV_8,
			SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,
			SPI_CR1_CPHA_CLK_TRANSITION_1,
			SPI_CR1_DFF_8BIT,
			SPI_CR1_MSBFIRST);

	spi_enable_software_slave_management(SPI1);
	/* spi_disable_software_slave_management(SPI1); */
	spi_disable_ss_output(SPI1);
	spi_set_nss_high(SPI1);
	/* spi_enable_ss_output(SPI1); */;
	spi_enable(SPI1);
}

// End
