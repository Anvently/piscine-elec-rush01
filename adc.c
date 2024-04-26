#include <adc.h>

uint8_t	readADC8(uint8_t mux)
{
	ADMUX |= (1 << REFS0); //Select external voltage reference
	ADMUX |= (1 << ADLAR);
	ADCSRA = (1 << ADEN); //Enable ADC
	ADCSRA |= 0b00000111;
	ADMUX &= ~(0b00001111);
	ADMUX |= mux; //Select ADC0
	ADCSRA |= (1 << ADSC); //Enable and start conversion
	while (ADCSRA & (1 << ADSC)); //Wait for the conversion to finish
	return (ADCH);
}

uint16_t	readADC16(uint8_t mux)
{
	ADMUX |= (1 << REFS0); //Select external voltage reference
	ADCSRA = (1 << ADEN); //Enable ADC
	ADCSRA |= 0b00000111;
	ADMUX &= ~(0b00001111);
	ADMUX |= mux; //Select ADC0
	ADCSRA |= (1 << ADSC); //Enable and start conversion
	while (ADCSRA & (1 << ADSC)); //Wait for the conversion to finish
	return (ADC);
}
