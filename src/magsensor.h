#pragma once

#include <stdint.h>
#include <array>




/* Based on MA782 */
class MagAngleSensor 
{
	private:
		std::uint32_t _spi;
		inline std::uint16_t _send_cmd(std::uint8_t cmd_header, std::uint8_t reg_val);
		std::array<int,4> arr;



	public:

		MagAngleSensor();
		void begin(std::uint32_t spi_base);
		std::uint16_t read_angle_raw();
		float read_angle_deg();
		float read_angle_rad();
		std::uint16_t read_register(std::uint16_t address);
		bool write_register(std::uint16_t address, std::uint16_t value);
		void store_single_register(std::uint16_t address);
		void store_all_registers();
		void clear_errors();


};
