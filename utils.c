#include <utils.h>
#include <expander.h>

void	print_buffer(void* buffer, uint16_t size)
{
	LOGI("");
	for (uint16_t i = 0; i < size; i++)
	{
		printHexa(((uint8_t*)buffer)[i]);
		print(" ", 0);
	}
	LOGI("");
}

/// @brief Copy n bytes from memory address src to memory address dest.
/// The memory areas must not overlap.
/// @param dest Memory address
/// @param src  Memory address
/// @param n Number of byte to copy from src to dest
/// @return Memory area pointed by dest
void	*ft_memcpy(void *dest, const void *src, uint16_t n)
{
	uint16_t	i;

	i = 0;
	if (!dest && ! src)
		return (NULL);
	while (i < n)
	{
		((unsigned char *) dest)[i] = ((unsigned char *) src)[i];
		i++;
	}
	return (dest);
}

/// @brief Compares the first n bytes from the memory areas s1 and s2.
/// @param s1
/// @param s2
/// @param n
/// @return difference between s1 and s2 where the first character differs
/// 0 if s1 and s2 are equal.
/// Negative value if s1 < s2.
/// Positive value if s1 > s2.
int	ft_memcmp(const void *s1, const void *s2, uint16_t n)
{
	uint16_t	i;

	i = 0;
	while (i < n)
	{
		if (((unsigned char *) s1)[i] != ((unsigned char *) s2)[i])
			return (((unsigned char *) s1)[i] - ((unsigned char *) s2)[i]);
		i++;
	}
	return (0);
}

/// @brief Return the number of non empty bytes in a string
/// @param s Null terminated string
/// @return Size if the given string, excluding the null terminating byte
uint16_t	ft_strlen(const char *s)
{
	uint16_t	i;

	i = 0;
	while (s[i])
		i++;
	return (i);
}

/// @brief Fill n bytes of given memory address pointed by s with given c byte.
/// @param s Address to fill
/// @param c Value to fill address with
/// @param n Number of byte to fill
/// @return s memory address
void	*ft_memset(void *s, int c, uint16_t n)
{
	uint16_t	i;

	i = 0;
	while (i < n)
		*((unsigned char *)(s + i++)) = (unsigned char) c;
	return (s);
}

/// @brief Compares the firt n characters of s1
/// and s2 using ascii value of character.
/// @param s1 Null terminated string
/// @param s2 Null terminated string
/// @return difference between s1 and s2 where the first character differs
/// 0 if s1 and s2 are equal.
/// Negative value if s1 < s2.
/// Positive value if s1 > s2.
int	ft_strncmp(const char *s1, const char *s2, uint16_t n)
{
	uint16_t	i;

	i = 0;
	while (i < n && s1[i] && s2[i] && s1[i] == s2[i])
		i++;
	if (i == n)
		return (0);
	return ((unsigned char) s1[i] - (unsigned char) s2[i]);
}

/// @brief Compares s1 and s2 using ascii value of character.
/// @param s1 Null terminated string
/// @param s2 Null terminated string
/// @return difference between s1 and s2 where the first character differs
/// 0 if s1 and s2 are equal.
/// Negative value if s1 < s2.
/// Positive value if s1 > s2.
int	ft_strcmp(const char *s1, const char *s2)
{
	while (*s1 && *s1 == *s2)
	{
		s1++;
		s2++;
	}
	return ((unsigned char) *s1 - (unsigned char) *s2);
}

void	ft_putnbr(uint16_t nb)
{
	if (nb <= 9)
		uart_send('0' + nb);
	else
	{
		ft_putnbr(nb / 10);
		ft_putnbr(nb % 10);
	}
}


bool	read_button1(void)
{
	static uint8_t	isPressed = false;

	if (BTN1_IS_PRESSED && isPressed == false) //if button is pressed for the first time
	{
		
		isPressed = true;
	}
	else if (BTN1_IS_PRESSED == false) //If state changes
		isPressed = false;
	return (false);
}

bool	read_button2(void)
{
	static uint8_t	isPressed = false;

	if (BTN2_IS_PRESSED && isPressed == false) //if button is pressed for the first time
	{	
		isPressed = true;
		return (true);
	}
	else if (BTN2_IS_PRESSED == false) //If state changes
		isPressed = false;
	return (false);
}

bool	read_button3(void)
{
	if (expander_read_port(EXPANDER_IPORT_0) & (1 << EXPANDER_BUTTON3_IO))
		return (false);
	return (true);
}

uint8_t	atoui_hex(char	str[2])
{
	uint8_t	value = 0;
	
	if (str[0] >= '0' && str[0] <= '9')
		value = (str[0] - '0') << 4;
	else if (str[0] >= 'A' && str[0] <= 'Z')
		value = (str[0] - 'A' + 10) << 4;
	else if (str[0] >= 'a' && str[0] <= 'z')
		value = (str[0] - 'a' + 10) << 4;
	if (str[1] >= '0' && str[1] <= '9')
		value |= (str[1] - '0');
	else if (str[1] >= 'A' && str[1] <= 'Z')
		value |= (str[1] - 'A' + 10);
	else if (str[1] >= 'a' && str[1] <= 'z')
		value |= (str[1] - 'A' + 10);
	return (value);
}

void	rgb_set(uint8_t r, uint8_t g, uint8_t b)
{
	//Set the duty cycle of each output
	OCR0A = (uint16_t)g * GREEN_FILTER / 10; //Set duty cycle for GREEN
	OCR0B = (uint16_t)r * RED_FILTER / 10; //Set duty cycle for RED
	OCR2B = (uint16_t)b * BLUE_FILTER / 10; //Set duty cycle for BLUE
}

void	rgb_wheel(uint8_t pos) {
	pos = 255 - pos;
	if (pos < 85) {
		rgb_set(255 - pos * 3, 0, pos * 3);
	} else if (pos < 170) {
		pos = pos - 85;
		rgb_set(0, pos * 3, 255 - pos * 3);
	} else {
		pos = pos - 170;
		rgb_set(pos * 3, 255 - pos * 3, 0);
	}
}

void	rgb_init(void)
{
	//TIMER 0 => OCOA / OCOB => RED AND GREEN

	OCR0A = DUTYGAP_TIMER0 * 0; //Set duty cycle for GREEN
	OCR0B = DUTYGAP_TIMER0 * 0; //Set duty cycle for RED
	TCCR0B |= (1 << CS00); //prescale to 256

	//Set operation mode to 1 (phase correct PWM, counting invert at 0XFF (255)) 
	TCCR0A |= (1 << WGM00);
	TCCR0A |= (1 << COM0A1) | (1 << COM0B1); //Set both output to  Clear OC0B on Compare Match when up-counting. Set OC0B on
											// Compare Match when down-counting.
	// TIMSK0 |= (1 << OCIE0A); //Enable interrupt

	//TIMER2 => OC2B => LED BLUE

	OCR2B = DUTYGAP_TIMER2 * 0; //Set duty cycle for BLUE
	TCCR2B |= (1 << CS22) | (1 << CS21); //| (1 << CS20); //prescale to 256

	//Set operation mode to 1 (phase correct PWM, counting invert at 0XFF (255)) 
	TCCR2A |= (1 << WGM20);

	TCCR2A |= (1 << COM2B1); //Set both OC2B => Clear OC2B on Compare Match when up-counting. Set OC2B on
							// Compare Match when down-counting.
}

void	print_value_leds(uint8_t value)
{
	PORTB &= 0b11101000;//set all PB leds to low
	//Check the 4 first bits of the value
	//Turn on the according led for each bit
	if (value & (1 << 0)) //Check bit 0
		PORTB |= (1 << PB0); //Turn on L1
	if (value & (1 << 1))
		PORTB |= (1 << PB1);
	if (value & (1 << 2))
		PORTB |= (1 << PB2);
	if (value & (1 << 3))
		PORTB |= (1 << PB4);
}

extern volatile uint16_t	MILLI_COUNTER;

void	timer_delay(uint16_t ms)
{
	uint16_t	counter = MILLI_COUNTER;
	while (MILLI_COUNTER - counter < ms);
}

static uint8_t	increment_nbr(uint16_t *dest, char c)
{
	uint8_t	add;

	add = c - '0';
	if (*dest > 9999 / 10)
		return (1);
	*dest = *dest * 10 + add;
	return (0);
}

/// @brief Check digit character [0-9]
/// @param c Character to check
/// @return Non-zero if true, zero otherwise
uint8_t	ft_isdigit(char c)
{
	if (c >= '0' && c <= '9')
		return (1);
	return (0);
}

/// @brief Convert the initial part of a string to an integer.
/// Format : \\[n spaces\\](+-)[n digit]
/// Check for overflow.
/// @param str String to convert
/// @param dest int receiving the conversion.
/// @return ```0``` if no error occured. ```1``` if overflow.
/// ```2``` if no digit was found at the beginning.
uint8_t	ft_strtoi(const char *str, uint16_t *dest)
{
	uint16_t	i;

	i = 0;
	*dest = 0;
	while (ft_isspace(str[i]))
		i++;
	if (!ft_isdigit(str[i]))
		return (2);
	while (ft_isdigit(str[i]))
		if (increment_nbr(dest, str[i++]))
			return (1);
	return (0);
}

uint8_t	ft_isspace(char c)
{
	if ((c >= '\b' && c <= '\r') || c == ' ')
		return (1);
	return (0);
}

uint8_t	parse_int(char** ptr, uint16_t *value, char sep)
{
	if (ft_strtoi(*ptr, value))
		return (1);
	while (**ptr && **ptr != sep)
		*ptr = *ptr + 1;
	return (0);
}

char	*skip_space(char **ptr)
{
	while (ft_isspace(**ptr))
		*ptr = *ptr + 1;
	return (*ptr);
}
