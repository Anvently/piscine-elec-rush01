#ifndef LOG_LVL
 #define LOG_LVL LOG_INFO
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <log.h>
#include <adc.h>
#include <expander.h>
#include <i2c.h>

#ifndef CPU_FREQ
 #define CPU_FREQ 16000000
#endif

#ifndef TRUE
 #define TRUE 1
#endif

#ifndef FALSE
 #define FALSE 0
#endif

#ifndef HEXCODE
 #define HEXCODE "0123456789ABCDEF"
#endif

#define BTN1_IS_PRESSED ((PIND & (1 << PD2)) == 0)
#define BTN2_IS_PRESSED ((PIND & (1 << PD4)) == 0)

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

extern const volatile uint8_t		segment_values[10];

static volatile	uint16_t	MILLI_COUNTER = 0;
static volatile	uint8_t		segmentIndex = 3;
static volatile	uint16_t	value = 9997;
static volatile	uint8_t		digit_number = 4;
const uint16_t				digit_value[4] = {1, 10, 100, 1000};

// Is triggered every ms (because Timer 1 is set to run at 1000Hz)
ISR (TIMER1_COMPA_vect)
{
	if (++MILLI_COUNTER % 10 == 0)
	{
		value = readADC16(MUX0);
		if (value >= 1000)
			digit_number = 4;
		else if (value >= 100)
			digit_number = 3;
		else if (value >= 10)
			digit_number = 2;
		else
			digit_number = 1;
		OCR0A = 255 / (digit_number + 3);
	}
}

//If 1 digit => f > 30Hz => 30ms
//If 2 digit => f > 60Hz => 16ms
//If 3 digit => f > 90Hz => 10ms
//If 4 digit => f > 120Hz => 8ms
ISR (TIMER0_COMPA_vect)
{
	expander_set_segment(segmentIndex, segment_values[value / digit_value[segmentIndex] % 10]);
	segmentIndex = (segmentIndex + 1) % digit_number;
}

int	main(void)
{
	/* ------------------------------ TIMER1 CONFIG ----------------------------- */
	//  Timer 1 is set to 1000Hz, every OCR1A match compare will generate an interrupt incrementing MILLI_COUNTER

	OCR1A = 8000; //Set TOP value
	TCCR1B |= (1 << CS10); //prescale to 1

	//Set operation mode to 11
	TCCR1A |= (1 << WGM11) | (1 << WGM10);
	TCCR1B |= (1 << WGM13);
	
	TIMSK1 |= (1 << OCIE1A);

	/* ------------------------------ TIMER0 CONFIG ----------------------------- */

	TCCR0A |= (1 << WGM00);
	TCCR0B |= (1 << WGM02) | (1 << CS02) | (1 << CS00);
	OCR0A = 255 / (digit_number + 3);
	TIMSK0 |= (1 << OCIE0A);

	/* --------------------------------- OUTPUT --------------------------------- */

	DDRB |= (1 << PB4) | (1 << PB2) | (1 << PB1) | (1 << PB0); //set leds to output
	/* -------------------------------- MAIN LOOP ------------------------------- */

	sei();

	LOGI("reset");

	i2c_init(100000, 0, I2C_MODE_MASTER_TX);
	expander_set_direction(EXPANDER_DFT_DDR0, EXPANDER_DFT_DDR1); //Set SW3 and LED5 to ouput
	expander_set_output(EXPANDER_DFT_PORT0, 0);
	while (1)
	{
	}
}