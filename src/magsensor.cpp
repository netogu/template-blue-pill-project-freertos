#include "magsensor.h"
#include <libopencm3/stm32/spi.h>



MagAngleSensor::MagAngleSensor(){
}

void MagAngleSensor::begin(uint32_t spi_base){
	_spi = spi_base;
}


uint16_t MagAngleSensor::read_angle_raw(){
	uint16_t raw_angle = 0;
	spi_enable(_spi);
	raw_angle = spi_xfer(_spi, 0x00) << 8;
	raw_angle |= spi_xfer(_spi, 0x00);
	spi_disable(_spi);
	return raw_angle;

}

float MagAngleSensor::read_angle(){
	uint16_t angle_raw = 0;
	float angle_degree;
	angle_raw = read_angle_raw();
	angle_degree = (angle_raw*360.0f)/65536.0f;
	return angle_degree;

} 
