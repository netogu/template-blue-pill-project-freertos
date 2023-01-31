#pragma once

#include "stdint.h"


/* Based on MA782 */
class MagAngleSensor 
{


	public:

		MagAngleSensor();
		void begin(uint32_t spi_base);
		float read_angle();
		uint16_t read_angle_raw();
		uint16_t read_register();
		uint16_t write_register();
		uint16_t store_single_register();
		uint16_t store_all_registers();
		uint16_t restore_all_registers();
		uint16_t clear_errors();


	private:
		uint32_t _spi;


};
