#include "magsensor.h"
#include <libopencm3/stm32/spi.h>

#define READ_ANGLE				0b000
#define CLEAR_ERR_FLG 		0b001
#define READ_REG					0b010
#define WRITE_REG					0b100
#define STORE_ALL				 	0b101
#define STORE_REG 		    0b111
#define CMD_HEADER(CMD_BITS, ADDRESS) (CMD_BITS << 5 | ADDRESS)

#define PI2 6.283185307	


MagAngleSensor::MagAngleSensor(){
	_spi = 0;
	 
}


uint16_t MagAngleSensor::_send_cmd(uint8_t cmd_header, uint8_t reg_val){

	uint16_t rx;

	spi_enable(_spi);
	spi_xfer(_spi, cmd_header);
	spi_xfer(_spi, reg_val);
	rx = spi_xfer(_spi, 0x00) << 8;
	rx |= spi_xfer(_spi, 0x00);

	return rx;

}


void MagAngleSensor::begin(uint32_t spi_base){
	_spi = spi_base;
}


uint16_t MagAngleSensor::read_angle_raw(){
	uint16_t angle_raw = 0;
	angle_raw = _send_cmd(CMD_HEADER(READ_ANGLE, 0x00), 0x00);
	return angle_raw;

}


double MagAngleSensor::read_angle_deg(){
	double angle;
	uint16_t angle_raw = 0;
	angle_raw = _send_cmd(CMD_HEADER(READ_ANGLE, 0x00), 0x00);
	angle = (angle_raw*360.0)/65536.0;

	return angle;

} 


double MagAngleSensor::read_angle_rad(){
	double angle;
	uint16_t angle_raw = 0;
	angle_raw = _send_cmd(CMD_HEADER(READ_ANGLE, 0x00), 0x00);
	angle = (angle_raw*PI2)/65536.0;

	return angle;

} 


uint16_t MagAngleSensor::read_register(uint16_t address){
	uint16_t value;
	value = _send_cmd(CMD_HEADER(READ_REG, address), 0x00);
	return value;

}


bool MagAngleSensor::write_register(uint16_t address, uint16_t value){

	uint16_t new_value;
	new_value = _send_cmd(CMD_HEADER(WRITE_REG, address), value);

	if(new_value == value)
		return true;
	else	
		return false;

}


void MagAngleSensor::store_single_register(uint16_t address){

	_send_cmd(CMD_HEADER(STORE_REG, address), 0x00);

}


void MagAngleSensor::store_all_registers(){

	_send_cmd(CMD_HEADER(STORE_ALL, 0x00), 0x00);

}


void MagAngleSensor::clear_errors(){

	_send_cmd(CMD_HEADER(CLEAR_ERR_FLG, 0x00), 0x00);

}











