#ifndef ADC_H
# define ADC_H

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

#define MUX_RV1 0x00
#define MUX_LDR 0x01
#define MUX_NTC 0x02

uint8_t		readADC8(uint8_t mux);
uint16_t	readADC16(uint8_t mux);

#endif