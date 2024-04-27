#include <rtc.h>
#include <utils.h>

volatile uint16_t	rtc_date[3] = {0};
volatile uint8_t	rtc_time[3] = {0};

uint8_t	rtc_get_datas(void)
{
	uint8_t	data = 0, century = 0;
	cli();
	i2c_init(10000, RTC_ADDRESS, I2C_MODE_MASTER_TX);
	i2c_start(RTC_ADDRESS, I2C_MODE_TX);
	i2c_write(0x02);
	i2c_stop();
	i2c_start(RTC_ADDRESS, I2C_MODE_RX);
	i2c_read(&data, 0);
	rtc_time[2] = (((data & 0b01110000) >> 4 )* 10) + (data & 0b00001111);
	i2c_read(&data, 0);
	rtc_time[1] = (((data & 0b01110000) >> 4 )* 10) + (data & 0b00001111);
	i2c_read(&data, 0);
	rtc_time[0] = (((data & 0b01110000) >> 4 )* 10) + (data & 0b00001111);
	i2c_read(&data, 0);
	rtc_date[2] = (((data & 0b01110000) >> 4 )* 10) + (data & 0b00001111);
	i2c_read(&data, 0);
	i2c_read(&data, 0);
	rtc_date[1] = (((data & 0b00010000) >> 4 )* 10) + (data & 0b00001111);
	century = data & (1 << 7);
	i2c_read(&data, 0);
	rtc_date[0] = (((data & 0b11110000) >> 4 )* 10) + (data & 0b00001111);
	if (century)
		rtc_date[0] += 2000;
	else
		rtc_date[0] += 1900;
	i2c_stop();
	sei();
}

void	rtc_set_datas(uint16_t* date, uint16_t* time)
{
	uint8_t data = 0;

	cli();
	i2c_init(10000, RTC_ADDRESS, I2C_MODE_MASTER_TX);
	i2c_start(RTC_ADDRESS, I2C_MODE_TX);
	i2c_write(0x02);

	data = (uint8_t)*(time + 2);
	i2c_write(((data / 10) << 4) | (data % 10)); //time second
	data = (uint8_t)*(time + 1);
	i2c_write(((data / 10) << 4) | (data % 10)); //time minute
	data = (uint8_t)*(time);
	i2c_write(((data / 10) << 4) | (data % 10)); //time hour
	data = (uint8_t)*(date + 2);
	i2c_write(((data / 10) << 4) | (data % 10)); //date day
	i2c_write(0); //date weekday 
	data = (uint8_t)*(date + 1);
	i2c_write(((data / 10) << 4) | (data % 10) | (*date > 1999 ? 1 << 7 : 0)); //date month
	data = (uint8_t)(*(date) % 100);
	i2c_write(((data / 10) << 4) | (data % 10)); //date year
	i2c_stop();
	sei();
}