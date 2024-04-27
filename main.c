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
#include <spi.h>
#include <aht20.h>
#include <utils.h>
#include <rtc.h>

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

#define BUTTON_1_BIT 0
#define BUTTON_2_BIT 1
#define BUTTON_3_BIT 2

#define DELAY_START 1000

extern const volatile uint8_t		segment_values[11];

volatile	uint16_t		MILLI_COUNTER = 0;
static volatile	uint8_t		segmentIndex = 3;
static volatile	uint16_t	value = 9997;
static volatile	uint8_t		digit_number = 4;
const uint16_t				digit_value[4] = {1, 10, 100, 1000};

extern volatile uint8_t		expander_port0;
extern volatile uint8_t		expander_port1;

static volatile uint8_t		button_state = 0;
static volatile uint8_t		isFloat = 0;
static volatile uint8_t		isTime = 0;

extern volatile float		ftemperature, fhumidity;
extern volatile uint16_t	rtc_date[3];
extern volatile uint8_t		rtc_time[3];


static volatile uint8_t		mode = 12;


static void	switch_mode(uint8_t newMode)
{
	if (mode == 3)
		isFloat = false;
	else if (mode == 4)
	{
		rgb_set(0, 0, 0);
		spi_init(SPI_MODE_MASTER);
		spi_set_leds(0, 0, 0);
		spi_init(SPI_MODE_DISABLE);
		digit_number = 0;
	}
	else if (mode == 6 || mode == 7 || mode == 8)
		isFloat = false;
	else if (mode == 9 || mode == 10 || mode == 11)
		isTime = false;
	mode = newMode;
	print_value_leds(mode);
}

static void	handle_button2(void)
{
	static uint8_t	isPressed = false;

	if (BTN2_IS_PRESSED && isPressed == false) //if button is pressed for the first time
	{	
		switch_mode((mode + 11) % 12);
		isPressed = true;
		expander_port0 &= ~(1 << EXPANDER_LED10_IO);
	}
	else if (BTN2_IS_PRESSED)
		expander_port0 &= ~(1 << EXPANDER_LED10_IO);
	else if (BTN2_IS_PRESSED == false) //If state changes
	{
		isPressed = false;
		expander_port0 |= (1 << EXPANDER_LED10_IO);
	}
}

static void	display_mode4(uint8_t segmentIndex)
{
	const uint8_t	values[4] = {0b01000000, 0b01011011, 0b01100110, 0b01000000};
	expander_set_segment((1 << (3 - segmentIndex)), values[segmentIndex]);
}

static void	flash_mode4(void)
{
	static uint8_t	index = 0;
	const uint32_t	colors[3] = {(uint32_t)BRIGHTNESS << 24 | 0xFF0000,\
								 (uint32_t)BRIGHTNESS << 24 | 0x00FF00,\
								 (uint32_t)BRIGHTNESS << 24 | 0x0000FF};
	// LOGI("PING");
	rgb_set((index == 0) * 255, (index == 1) * 255, (index == 2) * 255);
	if (mode != 4)
		return;
	spi_init(SPI_MODE_MASTER);
	spi_set_leds(colors[index], colors[index], colors[index]);
	spi_init(SPI_MODE_DISABLE);
	index = ++index % 3;
}

// Is triggered every ms (because Timer 1 is set to run at 1000Hz)
//If 1 digit => f > 100Hz => 10ms
//If 2 digit => f > 200Hz => 8ms
//If 3 digit => f > 300Hz => 6ms
//If 4 digit => f > 400Hz => 4ms
ISR (TIMER1_COMPA_vect)
{
	// if (++MILLI_COUNTER % 5 == 0)
	// {
		
	// }
	if (++MILLI_COUNTER % 20)
	{
		if (read_button3())
			expander_port0 &= ~(1 << EXPANDER_LED11_IO);
		else
			expander_port0 |= (1 << EXPANDER_LED11_IO);
		handle_button2();
	}
	if (MILLI_COUNTER % 1000 == 0 && mode == 4)
		flash_mode4();
	if (MILLI_COUNTER % (12 - 2 - digit_number * 2))
	{
		segmentIndex = (segmentIndex + 1) % digit_number;
		// if (mode == 4)
			// LOGI("PONG");
		if (mode == 4)
			display_mode4(segmentIndex);
		else if (digit_number && mode != 12)
			expander_set_segment((1 << (3 - segmentIndex)), segment_values[value / digit_value[segmentIndex] % 10] \
															| (((isFloat && segmentIndex == 1)
																|| (isTime && (segmentIndex == 2 || segmentIndex == 0))) << 7));
		else if (digit_number == 0)
			expander_set_segment(0b1111, 0);
		else
			expander_set_output(expander_port0, expander_port1);
	}
}


// ISR (PCINT2_vect) //Button 2
// {
// 	static uint8_t	click = 0;
// 	++click;
	
// 	if (click % 2)
// 		mode = (mode + 11) % 12;
// }

ISR (INT0_vect) //Button 1
{
	if (EICRA == 0b01) //If the button was pressed while the toggle detection mode is set
						// => that means event is actually caused by button release
	{
		EICRA = 0b10; //Set the detection mode back to falling edge 
		expander_port0 ^= (1 << EXPANDER_LED9_IO);
		switch_mode((mode + 1) % 12);
		return;
	}
	else//If the boutton had enough time to debounce
	{
		expander_port0 ^= (1 << EXPANDER_LED9_IO);
		EICRA = 0b01; //Set the detection mode to toggle to detect button release event
	}
}

static void	change_value(uint16_t newValue)
{
	value = newValue;
	if (newValue >= 1000)
		digit_number = 4;
	else if (newValue >= 100)
		digit_number = 3;
	else if (newValue >= 10)
		digit_number = 2;
	else
		digit_number = 1;
}

static void	routine(void)
{
	switch (mode)
	{
		case 0: //DISPLAY RV1 value
			change_value(readADC16(MUX0)); 
			break;

		case 1: //Display LDR value
			change_value(readADC16(MUX1));
			break;

		case 2: //Display NTC value
			change_value(readADC16(MUX2));
			break;
		
		case 3: //Read MCUtemp
			isFloat = true;
			digit_number = 3;
			change_value(readADCMCU());
			break;

		case 4: //42 + flash
			digit_number = 4;
			break;

		case 5:
			digit_number = 0;
			break;

		case 6:
			change_value((uint16_t)(ftemperature * 10));
			digit_number = 3;
			isFloat = true;
			aht20_measure();
			break;

		case 7:
			change_value((uint16_t)((ftemperature * 1.8 + 32) * 10));
			digit_number = 3;
			isFloat = true;
			aht20_measure();
			break;

		case 8:
			change_value((uint16_t)(fhumidity * 10));
			digit_number = 3;
			isFloat = true;
			aht20_measure();
			break;

		case 9:
			change_value((uint16_t)rtc_time[0] * 100 + rtc_time[1]);
			digit_number = 4;
			isTime = true;
			rtc_get_datas();
			break;

		case 10:
			change_value((uint16_t)rtc_date[2] * 100 + rtc_date[1]);
			digit_number = 4;
			isTime = true;
			rtc_get_datas();
			break;

		case 11:
			change_value((uint16_t)rtc_date[0]);
			digit_number = 4;
			rtc_get_datas();
			break;

		default:
			digit_number = 0;
			break;
	}
}

static void	startup(void)
{
	cli();
	expander_set_segment(0b1111, 255);
	sei();
	PORTB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4);
	timer_delay(DELAY_START);
	cli();
	expander_set_segment(0b1111, 0);
	sei();
	PORTB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB4));
}

static void	parser(char* buffer, uint16_t size)
{
	uint16_t	date[3] = {0}, time[3] = {0};
	LOGD("A line has been received");
	if (ft_strlen(buffer) != 19)
		return (LOGE("Invalid size"));
	char*	start = buffer;
	if (parse_int(&buffer, date + 2, '/') || buffer != start + 2 || date[2] == 0 || date[2] > 31)
		return (LOGE("Invalid day"));
	buffer++;
	if (parse_int(&buffer, date + 1, '/') || buffer != start + 5 || date[1] == 0 || date[1] > 12)
		return (LOGE("Invalid month"));
	buffer++;
	if (parse_int(&buffer, date, ' ') || buffer != start + 10)
		return (LOGE("Invalid year"));
	buffer++;
	if (parse_int(&buffer, time, ':') || buffer != start + 13 || time[0] > 23)
		return (LOGE("Invalid hour"));
	buffer++;
	if (parse_int(&buffer, time + 1, ':') || buffer != start + 16 || time[1] > 59)
		return (LOGE("Invalid minute"));
	buffer++;
	if (parse_int(&buffer, time + 2, '\0') || buffer != start + 19 || time[2] > 59)
		return (LOGE("Invalid second"));
	LOGD("Line is okay !");
	rtc_set_datas(date, time);
}

int	main(void)
{
	char	line[21] = {0};
	/* ------------------------------ TIMER1 CONFIG ----------------------------- */
	//  Timer 1 is set to 1000Hz, every OCR1A match compare will generate an interrupt incrementing MILLI_COUNTER

	OCR1A = 8000; //Set TOP value
	TCCR1B |= (1 << CS10); //prescale to 1

	//Set operation mode to 11
	TCCR1A |= (1 << WGM11) | (1 << WGM10);
	TCCR1B |= (1 << WGM13);

	/* --------------------------------- I/O --------------------------------- */

	i2c_init(100000, 0, I2C_MODE_MASTER_TX);
	DDRB |= (1 << PB4) | (1 << PB2) | (1 << PB1) | (1 << PB0); //set leds to output
	DDRD |= (1 << PD3) | (1 << PD5) | (1 << PD6);
	DDRD &= ~(1 << PD2);
	DDRD &= ~(1 << PD4);
	expander_set_direction(0b11111110, 255);

	/* -------------------------------- INTERRUPT ------------------------------- */

	TIMSK1 |= (1 << OCIE1A); //Timer 1 interrupt
	//Button 1
	EIMSK |= (1 << INT0);
	EICRA |= (1 << ISC01); //falling edge on INT0 pin will trigger INT0
	//Button 2
	// PCICR |= (1 << PCIE2); //enable pin change interrupt 2
	// PCMSK2 |= (1 << PCINT20);// | (PCINT18); //set mask for pcint20 (PD4) to 1
	uart_init_gnl_interrupt(line, 21, "cmd:", "0123456789/ :", parser);

	/* --------------------------------- RGB D5 --------------------------------- */

	rgb_init(); //Init timer 0 & 2
	rgb_set(0, 0, 0); //Set default duty cycle to 0

	// /* --------------------------------- SPI RGB -------------------------------- */

	spi_init(SPI_MODE_MASTER);
	spi_set_leds(0, 0, 0);
	spi_init(SPI_MODE_DISABLE);
	DDRB |= (1 << PB4) | (1 << PB2) | (1 << PB1) | (1 << PB0); //set leds to output

	/* -------------------------------- MAIN LOOP ------------------------------- */

	sei();

	aht20_calibrate();
	aht20_measure();
	LOGI("reset");
	startup();
	switch_mode(0);
	while (1)
	{
		routine();
	}
}