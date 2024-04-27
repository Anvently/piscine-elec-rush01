#include <rtc.h>
#include <utils.h>

volatile uint8_t	rtc_date[3] = {0};
volatile uint8_t	rtc_time[3] = {0};

uint8_t	rtc_get_datas(void)
{
	uint8_t	data = 0, data2 = 0;
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
	i2c_read(&data, 0);
	rtc_date[0] = (((data & 0b11110000) >> 4 )* 10) + (data & 0b00001111);
	i2c_stop();
	sei();
}