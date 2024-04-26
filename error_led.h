#ifndef ERROR_LED_H
# define ERROR_LED_H

#include <avr/interrupt.h>
#include <avr/io.h>

#ifndef CPU_FREQ
 #define CPU_FREQ 16000000
#endif

#ifndef TRUE
 #define TRUE 1
#endif

#ifndef FALSE
 #define FALSE 0
#endif

#define ERROR_LED_PORT PORTB
#define ERROR_LED_DDR DDRB
#define ERROR_LED_PIN PB0

uint8_t	error(void);

#endif