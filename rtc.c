#include <rtc.h>
#include <utils.h>

volatile uint8_t	rtc_date[3] = {0};
volatile uint8_t	rtc_time[3] = {0};

uint8_t	rtc_get_datas(void)
{
	cli();
	i2c_init(10000, RTC_ADDRESS, I2C_MODE_MASTER_TX);
	i2c_start(RTC_ADDRESS, I2C_MODE_TX);
	i2c_write(0x02);
	i2c_stop();
	i2c_start(RTC_ADDRESS, I2C_MODE_RX);
	i2c_read(rtc_time + 2, 0);
	i2c_read(rtc_time + 1, 0);
	i2c_read(rtc_time, 0);
	i2c_read(rtc_date + 2, 0);
	uint8_t	null_data;
	i2c_read(&null_data, 0);
	i2c_read(rtc_date + 1, 0);
	i2c_read(rtc_date, 1);
	i2c_stop();
	sei();
}