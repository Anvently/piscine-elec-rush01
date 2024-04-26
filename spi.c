#include <spi.h>


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


static volatile uint8_t	spiMode = SPI_MODE_DISABLE;


void	spi_init(uint8_t mode)
{
	if (spiMode == mode)
		return;
	if (mode == 0)
	{
		SPCR = 0;
		return;
	}
	switch (mode & SPI_MODE_MASTER)
	{
		case SPI_MODE_MASTER:
			SPI_DDRD_REGISTER = (1 << SPI_MOSI_PIN) | (1 << SPI_SCK_PIN) | (1 << SPI_SS_PIN);
			SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);//Enable interface and set master mode
			SPI_DDRD_REGISTER &= ~(1 << SPI_MISO_PIN);
			// SPI_PORT_REGISTER &= ~(1 << SPI_SS_PIN);
			break;
		
		default:
			SPI_DDRD_REGISTER &= ~(1 << SPI_MOSI_PIN | 1 << SPI_SCK_PIN | 1 << SPI_SS_PIN);
			SPI_DDRD_REGISTER |= (1 << SPI_MISO_PIN);
			// SPI_PORT_REGISTER &= ~(1<< SPI_SS_PIN);
			SPCR = (1 << SPE) | (1 << SPR0);
			break;
	}
	if (mode & (1 << SPI_MODE_INTERRUPT))
		SPCR |= (1 << SPIE);
	spiMode = mode;
}

uint8_t	spi_send(uint8_t byte)
{
	spi_init(SPI_MODE_MASTER);

	SPDR = byte;
	while (!SPI_FINISH); //Wait for the tranfer to be finished
	if (SPSR & (1 << WCOL)) //Check if a write collision occured
		return (SPI_ERROR_WCOLLISION);
	return (0);
}

void	spi_rgb_start_frame(void)
{
	for (uint8_t i = 0; i < 4; i++)
		spi_send(0);
}

void	spi_rgb_stop_frame(void)
{
	for (uint8_t i = 0; i < 4; i++)
		spi_send(255);
}

void	spi_rgb_color_frame(uint8_t brightness, uint8_t r, uint8_t g, uint8_t b)
{
	spi_send((0b111 << 5) | (brightness & 0b00011111)); //Send global frame
	spi_send(b); // Send blue
	spi_send(g); //Seend green
	spi_send(r); //Send red
}

void	spi_set_leds(uint32_t led1, uint32_t led2, uint32_t led3)
{
	// printHexa32(led1);
	// print(", ", 0);
	// printHexa32(led2);
	// print(", ", 0);
	// printHexa32(led3);
	// print(", ", 1);
	spi_rgb_start_frame();
	spi_rgb_color_frame(GET_BYTE(led1, 3) >> 3, GET_BYTE(led1, 2), GET_BYTE(led1, 1),GET_BYTE(led1, 0));
	spi_rgb_color_frame(GET_BYTE(led2, 3) >> 3, GET_BYTE(led2, 2), GET_BYTE(led2, 1),GET_BYTE(led2, 0));
	spi_rgb_color_frame(GET_BYTE(led3, 3) >> 3, GET_BYTE(led3, 2), GET_BYTE(led3, 1),GET_BYTE(led3, 0));
	spi_rgb_stop_frame();
}

void spi_wheel(uint8_t pos) {
	uint32_t value = 0;
	pos = 255 - pos;
	if (pos < 85) {
		value = ((uint32_t)BRIGHTNESS << 24) | ((uint32_t)(255 - pos * 3) << 16) |  (pos * 3);
		// set_rgb(255 - pos * 3, 0, pos * 3);
		
	} else if (pos < 170) {
		pos = pos - 85;
		value = ((uint32_t)BRIGHTNESS << 24) | ((uint32_t)(pos * 3) << 8) |  (255 - pos * 3);
		// set_rgb(0, pos * 3, 255 - pos * 3);
	} else {
		pos = pos - 170;
		value = ((uint32_t)BRIGHTNESS << 24) | ((uint32_t)(pos * 3) << 16) |  ((uint32_t)(255 - pos * 3) << 8);
		// set_rgb(pos * 3, 255 - pos * 3, 0);
	}
	spi_set_leds(value, value, value);
}
