#pragma once

#include "stdint.h"


/* Based on MA782 */
class MagAngleSensor 
{
	private:
		uint32_t _spi;
		inline uint16_t _send_cmd(uint8_t cmd_header, uint8_t reg_val);


	public:

		MagAngleSensor();
		void begin(uint32_t spi_base);
		double read_angle_deg();
		double read_angle_rad();
		uint16_t read_angle_raw();
		uint16_t read_register(uint16_t address);
		bool write_register(uint16_t address, uint16_t value);
		void store_single_register(uint16_t address);
		void store_all_registers();
		void clear_errors();


};
