#ifndef EXPANDER_H
# define EXPANDER_H

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/twi.h>
#include <error_led.h>
#include <log.h>
#include <util/delay.h>
#include <stdbool.h>
#include <i2c.h>

#ifndef CPU_FREQ
 #define CPU_FREQ 16000000
#endif

#define EXPANDER_ADDRESS_4 (0b0100 << 3)
#define EXPANDER_ADDRESS_3 (0b111)
#define EXPANDER_IPORT_0 0
#define EXPANDER_IPORT_1 1
#define EXPANDER_OPORT_0 2
#define EXPANDER_OPORT_1 3
#define EXPANDER_POL_0 4
#define EXPANDER_POL_1 5
#define EXPANDER_CONFIG_0 6
#define EXPANDER_CONFIG_1 7
#define EXPANDER_DFT_DDR0 0b11111110
#define EXPANDER_DFT_DDR1 0b11111111
#define EXPANDER_DFT_PORT0 0b11111110

#define EXPANDER_BUTTON3_IO 0
#define EXPANDER_LED11_IO 1
#define EXPANDER_LED10_IO 2
#define EXPANDER_LED9_IO 3

void		expander_set_direction(uint8_t port0, uint8_t port1);
uint16_t	expander_read_ports(void);
uint8_t		expander_read_port(uint8_t port);
void		expander_set_output(uint8_t port0, uint8_t port1);
void		expander_set_segment(uint8_t segment, uint8_t value);


#endif