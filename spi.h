#ifndef SPI_H
# define SPI_H

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/twi.h>
#include <error_led.h>
#include <log.h>
#include <util/delay.h>
#include <stdbool.h>

#ifndef CPU_FREQ
 #define CPU_FREQ 16000000
#endif

#define SPI_DDRD_REGISTER DDRB
#define SPI_PORT_REGISTER PORTB
#define SPI_MISO_PIN PB4
#define SPI_MOSI_PIN PB3
#define SPI_SCK_PIN PB5
#define SPI_SS_PIN PB2

#define SPI_FINISH (SPSR & (1 << SPIF))

#define SPI_ERROR_WCOLLISION 1

#define SPI_MODE_DISABLE 0
#define SPI_MODE_MASTER 1
#define SPI_MODE_INTERRUPT 4

#define BRIGHTNESS 0x10

#define GET_BYTE(value, b) (value >> (8 * b))

/*

IMPORTANT : Setting SS to HIGH means setting ss line to LOW (become SS is inverted)

How to initialize SPI as master
	// - SS toward proper slave pin must be set as output and pulled LOW
	-	SS toward proper slave must be set as input and pulled to HIGH
		- If driven low, then it would means another master is trying to enslave the interface 
			- SPCR:MSTR is cleared and MOSI and SCK will become inputs
				- it can be reset by software to renable Master mode
			- The SPIF interrupt flag is set
			=> THUS : if interrupt are globally enabled and SPI interrupt also, intterrupt routine should ALWAYS check
				that SCPR:MSTR is still on
	- SCPR :
		- SPE = 1
		- DORD (endianess) = 0
		- MSTR = 1
		- CPOL / CPHA = 0
		- SPR1/0 => ?
	- SPSR
		- may want to check WCOL(write collision flag) which is the collision flag register
			- it is cleared once data register has been accessed
	- SPDR : data register 

*/

uint8_t	spi_send(uint8_t byte);
void	spi_init(uint8_t mode);

void	spi_rgb_start_frame(void);
void	spi_rgb_stop_frame(void);
void	spi_rgb_color_frame(uint8_t brightness, uint8_t r, uint8_t g, uint8_t b);

void	spi_set_leds(uint32_t led1, uint32_t led2, uint32_t led3);

void 	spi_wheel(uint8_t pos);

#endif