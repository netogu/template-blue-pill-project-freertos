#include "magsensor.h"
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/gpio.h>


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


MagAngleSensor::MagAngleSensor(): _spi(0),_cs_port(0),_cs_pin(0){
	 
}


inline uint16_t MagAngleSensor::_send_cmd(uint8_t cmd_header, uint8_t reg_val){

	uint16_t rx;

	spi_set_dff_16bit(_spi);
	spi_enable(_spi);
	spi_xfer(_spi,cmd_header << 8 | reg_val);
	rx = spi_xfer(_spi, 0x0000);
	spi_set_dff_8bit(_spi);

	return rx;

}

uint16_t MagAngleSensor::read_angle_raw(){
	uint16_t angle_raw = 0;
	spi_set_dff_16bit(_spi);
	gpio_clear(_cs_port, _cs_pin);
	angle_raw = spi_xfer(_spi, 0x0000);
	gpio_set(_cs_port,_cs_pin);
	spi_set_dff_8bit(_spi);
	return angle_raw;

}


void MagAngleSensor::begin(uint32_t spi_base, uint32_t cs_port, uint32_t cs_pin){
	_spi = spi_base;
	_cs_port = cs_port;
	_cs_pin = cs_pin;

}




float MagAngleSensor::read_angle_deg(){
	float angle;
	uint16_t angle_raw = 0;
	angle_raw = read_angle_raw();
	angle = (angle_raw*360.0)/65536.0;

	return angle;

} 


float MagAngleSensor::read_angle_rad(){
	float angle;
	uint16_t angle_raw = 0;
	angle_raw = read_angle_raw();
	angle = (angle_raw*PI2)/65536.0f;

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











