#pragma once

#include <stdint.h>




/* Based on MA782 */
class MagAngleSensor 
{
	private:
		uint32_t _spi;
		uint32_t _cs_port;
		uint32_t _cs_pin;
		inline uint16_t _send_cmd(uint8_t cmd_header, uint8_t reg_val);



	public:

		MagAngleSensor();
		void begin(uint32_t spi_base,uint32_t cs_port, uint32_t cs_pin);
		uint16_t read_angle_raw();
		float read_angle_deg();
		float read_angle_rad();
		uint16_t read_register(uint16_t address);
		bool write_register(uint16_t address, uint16_t value);
		void store_single_register(uint16_t address);
		void store_all_registers();
		void clear_errors();


};
