#include <aht20.h>
#include <utils.h>

static uint32_t	temperature = 0, humidity = 0;
static uint8_t	crc = 0, isInit = FALSE;

volatile float	ftemperature = 0.f, fhumidity = 0.f;

void	aht20_calibrate()
{
	cli();
	if (isInit == FALSE)
		i2c_init(100000, 0, I2C_MODE_MASTER_TX);
	LOGD("sending calibration command");
	i2c_start(0x38, I2C_MODE_TX);
	i2c_write(0xbe);
	i2c_write(0x08);
	i2c_write(0x00);
	i2c_stop();
	sei();
	timer_delay(10);
}

void	aht20_soft_reset(void)
{
	cli();
	if (isInit == FALSE)
		i2c_init(100000, 0, I2C_MODE_MASTER_TX);
	LOGD("resetting sensor");
	i2c_start(0x38, I2C_MODE_TX);
	i2c_write(0b10111010);
	i2c_stop();
	sei();
	timer_delay(40);
}

uint8_t	aht20_get_status()
{
	cli();
	if (isInit == FALSE)
		i2c_init(100000, 0, I2C_MODE_MASTER_TX);
	uint8_t	status = 0;
	i2c_start(0x38, I2C_MODE_RX);
	i2c_read(&status, 1);
	i2c_stop();
	sei();
	return (status);
}

uint8_t	aht20_is_calibrated(void)
{
	cli();
	if (isInit == FALSE)
		i2c_init(100000, 0, I2C_MODE_MASTER_TX);
	if (aht20_get_status() & (1 << 3))
	{
		sei();
		return (TRUE);
	}
	sei();
	return (FALSE);
}

uint8_t	aht20_is_buzzy(void)
{
	cli();
	if (isInit == FALSE)
		i2c_init(100000, 0, I2C_MODE_MASTER_TX);

	if (aht20_get_status() & (1 << 7))
	{
		sei();
		return (TRUE);
	}
	sei();
	return (FALSE);
}

int8_t	aht20_measure(void)
{
	static uint8_t	retry = 0;
	uint8_t		byte;

	cli();
	if (isInit == FALSE)
		i2c_init(100000, 0, I2C_MODE_MASTER_TX);

	humidity = 0;
	temperature = 0;

	LOGD("sending mesurement command");
	i2c_start(0x38, I2C_MODE_TX);
	i2c_write(0xAC);
	i2c_write(0x33);
	i2c_write(0x00);
	i2c_stop();
	sei();
	timer_delay(80);

	cli();
	i2c_start(0x38, I2C_MODE_RX);
	i2c_read(&byte, 0);
	if ((byte & (1 << 3)) == 0)
	{
		sei();	
		return (LOGI("Measurement failed, captor is uncalibrated"), AHT20_ERROR_UNCALIBRATED);
	}
	if ((byte & (1 << 7)) == 1 && retry++ < MAX_RETRY)
	{
		sei();
		LOGI("Captor buzzy, retrying...");
		timer_delay(10);
		return (aht20_measure());
	}
	else if (retry >= MAX_RETRY)
	{
		sei();
		return (AHT20_ERROR_BUZZY);
	}
	i2c_read(&byte, 0);
	humidity = byte;
	i2c_read(&byte, 0);
	humidity = (humidity << 8) | byte;
	i2c_read(&byte, 0);
	humidity = (humidity << 4) | (byte & 0b11110000);
	temperature = byte & 0b00001111;
	i2c_read(&byte, 0);
	temperature = (temperature << 8) | byte;
	i2c_read(&byte, 0); //if crc is not requested, send NACK
	temperature = (temperature << 8) | byte;
	i2c_read(&crc, 1);
	i2c_stop();
	sei();
	retry = 0;
	ftemperature = (float)(((float)temperature / ((uint32_t)1 << 20) * 200) - 50);
	fhumidity = (float)((float)humidity / ((uint32_t)1 << 20) * 100);
	return (0);
}

float	aht20_get_temperature(void)
{
	float	averageTemp = 0;
	for (uint8_t i = 0; i < 3; i++)
	{
		if (aht20_measure() == AHT20_ERROR_UNCALIBRATED)
		{
			aht20_calibrate();
			if (aht20_measure())
				return (LOGE("Could not read from sensor"), 1);
		}
		averageTemp += (float)(((float)temperature / ((uint32_t)1 << 20) * 200) - 50);
	}
	return (averageTemp / 3);
}

int16_t	aht20_get_humidity(void)
{
	float	averageHumid = 0;
	for (uint8_t i = 0; i < 3; i++)
	{
		if (aht20_measure() == AHT20_ERROR_UNCALIBRATED)
		{
			aht20_calibrate();
			if (aht20_measure())
				return (LOGE("Could not read from sensor"), 1);
		}
		averageHumid += (float)((float)humidity / ((uint32_t)1 << 20) * 100);
	}
	return (averageHumid / 3);	
}

void	aht20_display_readings(void)
{
	char temp_str[8] = {0};
	char humid_str[8] = {0};
	
	float temp = aht20_get_temperature();
	float humid = aht20_get_humidity();

	print("Temperature: ", 0);
	dtostrf((double)temp, 0, 1, temp_str);
	print(temp_str, 0);
	print("°C, Humidity: ", 0);
	dtostrf((double)humid, 4, 1, humid_str);
	print(humid_str, 0);
	print("%", 1);
}
