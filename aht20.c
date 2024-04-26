/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   aht20.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: npirard <npirard@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/04/20 13:20:56 by npirard           #+#    #+#             */
/*   Updated: 2024/04/26 18:17:09 by npirard          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <aht20.h>

static uint32_t	temperature = 0, humidity = 0;
static uint8_t	crc = 0, isInit = FALSE;

void	aht20_calibrate()
{
	if (isInit == FALSE)
		i2c_init(100000, 0, I2C_MODE_MASTER_TX);

	LOGI("sending calibration command");
	i2c_start(0x38, I2C_MODE_TX);
	i2c_write(0xbe);
	i2c_write(0x08);
	i2c_write(0x00);
	i2c_stop();
	_delay_ms(10);
}

void	aht20_soft_reset(void)
{
	if (isInit == FALSE)
		i2c_init(100000, 0, I2C_MODE_MASTER_TX);

	LOGI("resetting sensor");
	i2c_start(0x38, I2C_MODE_TX);
	i2c_write(0b10111010);
	i2c_stop();
	_delay_ms(40);
}

uint8_t	aht20_get_status()
{
	if (isInit == FALSE)
		i2c_init(100000, 0, I2C_MODE_MASTER_TX);

	uint8_t	status = 0;
	i2c_start(0x38, I2C_MODE_RX);
	i2c_read(&status, 1);
	i2c_stop();
	return (status);
}

uint8_t	aht20_is_calibrated(void)
{
	if (isInit == FALSE)
		i2c_init(100000, 0, I2C_MODE_MASTER_TX);

	if (aht20_get_status() & (1 << 3))
		return (TRUE);
	return (FALSE);
}

uint8_t	aht20_is_buzzy(void)
{
	if (isInit == FALSE)
		i2c_init(100000, 0, I2C_MODE_MASTER_TX);

	if (aht20_get_status() & (1 << 7))
		return (TRUE);
	return (FALSE);
}

int8_t	aht20_measure(void)
{
	static uint8_t	retry = 0;
	uint8_t		byte;

	if (isInit == FALSE)
		i2c_init(100000, 0, I2C_MODE_MASTER_TX);

	humidity = 0;
	temperature = 0;

	LOGI("sending mesurement command");
	i2c_start(0x38, I2C_MODE_TX);
	i2c_write(0xAC);
	i2c_write(0x33);
	i2c_write(0x00);
	i2c_stop();
	_delay_ms(80);

	i2c_start(0x38, I2C_MODE_RX);
	i2c_read(&byte, 0);
	if ((byte & (1 << 3)) == 0)
		return (LOGI("Measurement failed, captor is uncalibrated"), AHT20_ERROR_UNCALIBRATED);
	if ((byte & (1 << 7)) == 1 && retry++ < MAX_RETRY)
	{
		LOGI("Captor buzzy, retrying...");
		_delay_ms(10);
		return (aht20_measure());
	}
	else if (retry >= MAX_RETRY)
		return (AHT20_ERROR_BUZZY);
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
	retry = 0;
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
