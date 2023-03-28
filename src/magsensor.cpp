#include "magsensor.h"
#include <libopencm3/stm32/spi.h>


//TODO change to enum
#define READ_ANGLE				0b000
#define CLEAR_ERR_FLG 		0b001
#define READ_REG					0b010
#define WRITE_REG					0b100
#define STORE_ALL				 	0b101
#define STORE_REG 		    0b111
//TODO change to struct + bit field
#define CMD_HEADER(CMD_BITS, ADDRESS) (CMD_BITS << 5 | ADDRESS)

#define PI2 6.283185307	


MagAngleSensor::MagAngleSensor(): _spi(0){
	 
}


inline std::uint16_t MagAngleSensor::_send_cmd(std::uint8_t cmd_header, std::uint8_t reg_val){

	std::uint16_t rx;

	spi_set_dff_16bit(_spi);
	spi_enable(_spi);
	spi_xfer(_spi,cmd_header << 8 | reg_val);
	rx = spi_xfer(_spi, 0x0000);
	spi_set_dff_8bit(_spi);

	return rx;

}

std::uint16_t MagAngleSensor::read_angle_raw(){
	std::uint16_t angle_raw = 0;
	spi_set_dff_16bit(_spi);
	spi_enable(_spi);
	angle_raw = spi_xfer(_spi, 0x0000);
	spi_disable(_spi);
	spi_set_dff_8bit(_spi);
	return angle_raw;

}


void MagAngleSensor::begin(std::uint32_t spi_base){
	_spi = spi_base;
}




float MagAngleSensor::read_angle_deg(){
	float angle;
	std::uint16_t angle_raw = 0;
	angle_raw = read_angle_raw();
	angle = (angle_raw*360.0)/65536.0;

	return angle;

} 


float MagAngleSensor::read_angle_rad(){
	float angle;
	std::uint16_t angle_raw = 0;
	angle_raw = read_angle_raw();
	angle = (angle_raw*PI2)/65536.0f;

	return angle;

} 


std::uint16_t MagAngleSensor::read_register(std::uint16_t address){
	std::uint16_t value;
	value = _send_cmd(CMD_HEADER(READ_REG, address), 0x00);
	return value;

}


bool MagAngleSensor::write_register(std::uint16_t address, std::uint16_t value){

	std::uint16_t new_value;
	new_value = _send_cmd(CMD_HEADER(WRITE_REG, address), value);

	if(new_value == value)
		return true;
	else	
		return false;

}


void MagAngleSensor::store_single_register(std::uint16_t address){

	_send_cmd(CMD_HEADER(STORE_REG, address), 0x00);

}


void MagAngleSensor::store_all_registers(){

	_send_cmd(CMD_HEADER(STORE_ALL, 0x00), 0x00);

}


void MagAngleSensor::clear_errors(){

	_send_cmd(CMD_HEADER(CLEAR_ERR_FLG, 0x00), 0x00);

}











