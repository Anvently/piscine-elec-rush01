#include <expander.h>

const volatile uint8_t	segment_values[10] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, \
					0b01100110, 0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111};

void	expander_set_direction(uint8_t port0, uint8_t port1)
{
	i2c_start(EXPANDER_ADDRESS_4 | EXPANDER_ADDRESS_3, I2C_MODE_TX);
	i2c_write(EXPANDER_CONFIG_0);
	i2c_write(~port0);
	i2c_write(~port1);
	i2c_stop();
}

/// @brief Return both port value on 16 bits [port1] [port0]
uint16_t	expander_read_ports(void)
{
	uint16_t	data = 0;
	i2c_start(EXPANDER_ADDRESS_4 | EXPANDER_ADDRESS_3, I2C_MODE_TX);
	i2c_write(EXPANDER_IPORT_1);
	i2c_start(EXPANDER_ADDRESS_4 | EXPANDER_ADDRESS_3, I2C_MODE_RX);
	i2c_read((uint8_t*)&data, 0);
	i2c_read((uint8_t*)&data + 1, 1);
	i2c_stop();
	return (data);
}

uint8_t	expander_read_port(uint8_t port)
{
	uint8_t	data = 0;
	i2c_start(EXPANDER_ADDRESS_4 | EXPANDER_ADDRESS_3, I2C_MODE_TX);
	i2c_write(port);
	i2c_start(EXPANDER_ADDRESS_4 | EXPANDER_ADDRESS_3, I2C_MODE_RX);
	i2c_read(&data, 1);
	i2c_stop();
	return (data);
}

void	expander_set_output(uint8_t port0, uint8_t port1)
{
	i2c_start(EXPANDER_ADDRESS_4 | EXPANDER_ADDRESS_3, I2C_MODE_TX);
	i2c_write(EXPANDER_OPORT_0);
	i2c_write(port0);
	i2c_write(port1);
	i2c_stop();
}

void	expander_set_segment(uint8_t segment, uint8_t value)
{
	uint8_t port0 = EXPANDER_DFT_PORT0 & ~(1 << (7 - segment));
	expander_set_output(EXPANDER_DFT_PORT0, 0);
	expander_set_output(port0, value);
}
