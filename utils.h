#ifndef UTILS_H
# define UTILS_H

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/twi.h>
#include <error_led.h>
#include <log.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef CPU_FREQ
 #define CPU_FREQ 16000000
#endif

#ifndef HEXCODE
 #define HEXCODE "0123456789ABCDEF"
#endif

#define DUTYGAP_TIMER0 25
#define DUTYGAP_TIMER2 25

#define RGB_LED_RED 0
#define RGB_LED_GREEN 1
#define RGB_LED_BLUE 2

#define RGB_LED_RED_MSK (1 << PD5)
#define RGB_LED_BLUE_MSK (1 << PD3)
#define RGB_LED_GREEN_MSK (1 << PD6)

#define BLUE_FILTER 7 //60%
#define GREEN_FILTER 7
#define RED_FILTER 10 //100%

#define BTN1_IS_PRESSED ((PIND & (1 << PD2)) == 0)
#define BTN2_IS_PRESSED ((PIND & (1 << PD4)) == 0)

void		print_buffer(void* buffer, uint16_t size);
void		*ft_memcpy(void *dest, const void *src, uint16_t n);
int			ft_memcmp(const void *s1, const void *s2, uint16_t n);
uint16_t	ft_strlen(const char *s);
void		*ft_memset(void *s, int c, uint16_t n);
int			ft_strncmp(const char *s1, const char *s2, uint16_t n);
int			ft_strcmp(const char *s1, const char *s2);
void		ft_putnbr(uint16_t nb);

bool		read_button1(void);
bool		read_button2(void);
bool		read_button3(void);
void		print_value_leds(uint8_t value);

uint8_t		atoui_hex(char	str[2]);

void		rgb_init(void);
void		rgb_wheel(uint8_t pos);
void		rgb_set(uint8_t r, uint8_t g, uint8_t b);

uint8_t		parse_int(char** ptr, uint16_t *value, char sep);
char		*skip_space(char **ptr);
uint8_t		ft_isspace(char c);
uint8_t		ft_strtoi(const char *str, uint16_t *dest);

void		timer_delay(uint16_t ms);

#endif