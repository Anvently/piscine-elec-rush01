#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/twi.h>
#include <error_led.h>
#include <util/delay.h>
#include <i2c.h>
#include <log.h>
#include <stdlib.h>

#ifndef CPU_FREQ
 #define CPU_FREQ 16000000
#endif

#ifndef TRUE
 #define TRUE 1
#endif

#ifndef FALSE
 #define FALSE 0
#endif

#define RTC_ADDRESS 0b1010001

uint8_t	rtc_get_datas(void);
void	rtc_set_datas(uint16_t* date, uint16_t* time);


