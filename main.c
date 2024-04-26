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

static volatile uint8_t		expander_port0 = EXPANDER_DFT_PORT0;
static volatile uint8_t		expander_port1 = 0;

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

//1000HZ timer => every ms
ISR (TIMER0_COMPA_vect)
{
	// expander_set_segment(segmentIndex, segment_values[value / digit_value[segmentIndex] % 10]);
	// segmentIndex = (segmentIndex + 1) % digit_number;
}

static void	startup(void)
{
	expander_port0 &= ~(0b1111 << 4);
	expander_port1 = 0b11111111;
	expander_set_output(expander_port0, expander_port1);
	PORTB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4);
	_delay_ms(3000);
	expander_port0 |= (1111 << 4);
	expander_port1 = 0;
	expander_set_output(expander_port0, expander_port1);
	PORTB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4));
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

	/* --------------------------------- I/O --------------------------------- */

	i2c_init(100000, 0, I2C_MODE_MASTER_TX);
	DDRB |= (1 << PB4) | (1 << PB2) | (1 << PB1) | (1 << PB0); //set leds to output
	DDRD &= ~((1 << PD2) | (1 << PD4));
	expander_set_direction(0b11111110, 255);

	/* -------------------------------- MAIN LOOP ------------------------------- */

	sei();

	LOGI("reset");
	startup();
	while (1)
	{
	}
}