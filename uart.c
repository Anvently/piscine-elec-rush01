#include <uart.h>

static uint8_t	isInit = FALSE;

static uint8_t	uart_mode = 0;

/// @brief Configure UART
/// @param mode uart mode, 4 last bits set the synchronisation mode : ASYNC, SYNC, MASTER.
/// Bit 5 set TX enable, bit 4 set RX enable
/// @param baudrate target baudrate
/// @param dataBits number of bit inside data frame, from 5 to 8 (or 9 if no parityBit)
/// @param parityBit ```0``` or ```1``` to toggle parityBit usage
/// @param stopBits number of stop bit, from ```1``` to ```2```
uint8_t	uart_init(uint8_t mode, uint32_t baudrate, uint8_t dataBits, uint8_t parityBit, uint8_t stopBits)
{
	if (isInit)
		return (0);
	if ((dataBits < 5 || (dataBits > (8 + !parityBit)))
		|| (stopBits > 2))
		return (1);

	//Set uart synchronisation mode
	UCSR0C |= ((mode & 0b00001111) << UMSEL00);
	uart_mode = mode;
 
	//Set number of data bits
	if (dataBits == 9) //If 9 bits, 2 register to set
	{
		UCSR0C |= (0b11 << UCSZ00);
		UCSR0B |= (1 << UCSZ02);
	}
	else
		UCSR0C |= ((dataBits - 5) << UCSZ00);
	
	//Set parity bit
	UCSR0C |= (parityBit << UPM00);

	//Set stopBits
	UCSR0C |= ((stopBits - 1) << USBS0); // - 1 because 2 stopbits = 01 and 1 stopbit = 00

	//Set baudrate
	uint16_t ubrr = 0;
	if ((mode & 0b00001111) == UART_MODE_ASYNC)
		ubrr = (CPU_FREQ + (8 * baudrate)) / (16 * baudrate) - 1;
	else if ((mode & 0b00001111) == UART_MODE_SYNC)
		ubrr = (CPU_FREQ + (4 * baudrate)) / (8 * baudrate) - 1;
	else if ((mode & 0b00001111) == UART_MODE_MASTER)
		ubrr = (CPU_FREQ + baudrate) / (2 * baudrate) - 1;
	if ((ubrr >> 12) > 0) //Check for overflow for ubrr
		return (1);
	UBRR0L = (uint8_t) ubrr;
	UBRR0H = (uint8_t)(ubrr >> 8);

	//set TX enable
	UCSR0B |= (((mode & UART_TX_ENABLE) >> 5) << TXEN0);

	//set RX enable
	UCSR0B |= (((mode & UART_RX_ENABLE) >> 4) << RXEN0);

	isInit = TRUE;

	return (0);
}

void	uart_send(uint16_t data)
{
	while (UART_TX_COMPLETE == FALSE); //While previous frame is not transferred yet
	
	if (UART_DATA_BITS == 9) //if frame have 9 data bits
	{
		UCSR0B &= ~(1 << TXB80); //Clear previous bit
		UCSR0B |= (((uint8_t) data & (0x0100)) << TXB80); //Read 9th bit of data and assign it to TXB80
	}
	UDR0 = (uint8_t) data;
}

void	uart_printstr(const char* str, uint8_t newline)
{
	for (uint8_t i = 0; str[i]; i++)
		uart_send((uint8_t) str[i]);
	if (newline)
	{
		uart_send('\r'); //to display correctly on screen
		uart_send('\n');
	}
}

/// @brief Return 
/// @param  
/// @return 
int32_t	uart_rx_16(void)
{
	while (UART_RX_COMPLETE == FALSE); //Wait until something was received
	uint8_t		status = UCSR0A;
	uint16_t	data;

	cli(); //Disable interrupt because 16bits manipulation is going to happen

	if (UART_DATA_BITS == 9)
		data = ((UCSR0B & RXB80) << 7) | UDR0; //Retrieve the 9th bit in UCSR0B and add it to the other 8 bits
	else
		data = UDR0; //Read RX_BUFFER

	sei();

	//Check error flags
	if (status & (1 << FE0))
		return (UART_ERROR_STOP_BIT);
	if (status & (1 << DOR0))
		return (UART_ERROR_DATA_LOSS);
	if (status & (1 << UPE0))
		return (UART_ERROR_PARITY);

	return (data);
}

char	uart_rx(void)
{
	return ((char) uart_rx_16());
}

static bool	isValid(const char* restricted, const char c)
{
	uint16_t i = 0;
	while (restricted[i] && restricted[i] != c)
		i++;
	if (restricted[i] == c)
		return (true);
	return (false);
}

static volatile char*		gnl_buffer;
static volatile uint16_t	gnl_buffer_size;
static const char*			gnl_prompt;
static const char*			gnl_restricted;
static void 				(*gnl_handler)(char*, uint16_t);

void	uart_init_gnl_interrupt(volatile char *buffer, volatile uint16_t size, const char* prompt, const char *restricted, void (*handler)(char*, uint16_t))
{
	if (!isInit || !(uart_mode & (UART_RX_ENABLE)))
	{
		if (uart_init(UART_MODE_ASYNC | UART_TX_ENABLE | UART_RX_ENABLE, UART_BAUDRATE, 
			UART_DATA_BITS, UART_PARITY_BIT_E, UART_STOP_BITS))
		{
			error();
			return;
		}
		UCSR0B |= (1 << RXCIE0); //enable interrupt receive
	}
	gnl_buffer = buffer;
	gnl_buffer_size = size;
	gnl_prompt = prompt;
	gnl_restricted = restricted;
	gnl_handler = handler;
	uart_printstr(prompt, 0);
}

ISR (USART_RX_vect)
{
	uart_gnl(gnl_buffer, gnl_buffer_size, gnl_prompt, gnl_restricted, gnl_handler);
}

void	uart_gnl(volatile char* buffer, volatile uint16_t size, const char* prompt, const char* restricted, void (*handler)(char*, uint16_t))
{
	char			c = 0;
	static uint16_t	index = 0;


	if (!isInit || !(uart_mode & (UART_RX_ENABLE)))
	{
		if (uart_init(UART_MODE_ASYNC | UART_TX_ENABLE | UART_RX_ENABLE, UART_BAUDRATE, 
			UART_DATA_BITS, UART_PARITY_BIT_E, UART_STOP_BITS))
		{
			error();
			return;
		}
	}

	do
	{
		c = uart_rx();
		if (c < 0)
			error();
		else if (c == 127 && index > 0) //If char is DELETE
		{
			buffer[index--] = '\0';
			uart_printstr("\b \b", 0);
		}
		else if (c == '\r')
			uart_printstr("", 1);
		else if (c < 32 || c > 126 || index + 1 == size) {}
		else if (index + 1 < size && (!restricted || isValid(restricted, c)))
		{
			buffer[index++] = c;
			uart_send(c);
		}
	} while (c != '\r' && (UCSR0B & (1 << RXCIE0) == 0));
	if (c == '\r')
	{
		buffer[index] = '\0';
		index = 0;
		if (handler)
			handler(buffer, size);
		uart_printstr(prompt, 0);
	}
}
