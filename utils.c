#include <utils.h>

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

bool	read_button1(void)
{
	static uint8_t	isPressed = FALSE;

	if (BTN1_IS_PRESSED && isPressed == FALSE) //if button is pressed for the first time
	{
		
		isPressed = TRUE;
	}
	else if (BTN1_IS_PRESSED == FALSE) //If state changes
		isPressed = FALSE;
	
}

bool	read_button2(void)
{
	static uint8_t	isPressed = FALSE;

	if (BTN2_IS_PRESSED && isPressed == FALSE) //if button is pressed for the first time
	{
		
		isPressed = TRUE;
	}
	else if (BTN2_IS_PRESSED == FALSE) //If state changes
		isPressed = FALSE;
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
	OCR0A = DUTYGAP_TIMER0 * ((g / 25 * GREEN_FILTER) / 10); //Set duty cycle for GREEN
	OCR0B = DUTYGAP_TIMER0 * ((r / 25 * RED_FILTER) / 10); //Set duty cycle for RED
	OCR2B = DUTYGAP_TIMER2 * ((b / 25 * BLUE_FILTER) / 10); //Set duty cycle for BLUE
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
