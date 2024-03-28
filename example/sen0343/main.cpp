#include <stdio.h>

#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <pico/bootrom.h>

#include <hardware/i2c.h>

#define I2C1_SDA_PIN 6
#define I2C1_SCL_PIN 7

#define MAX_PRESURE 600 ///< Maximum value of pressure range(Pa)
#define MIN_PRESURE 600 ///< Minimum value of pressure range(Pa)

#define MAX_TEMP 85.0 ///< Highest value of temperature range(℃)
#define MIN_TEMP 40.0 ///< Lowest value of temperature range(℃)

uint8_t address = 0x00;

int i2c_config(i2c_inst_t *inst)
{
	uint8_t config[3] = {0xAA, 0x00, 0x80};
	int res = i2c_write_timeout_us(inst, address, config, 3, false, 1000);
	if (res == PICO_ERROR_TIMEOUT)
	{
		// printf("I2C Config Timeout\n");
		return -1;
	}
	else
	{
		// printf("I2C Config Success (%d)\n", res);
		sleep_ms(30);
		return 0;
	}
}

int i2c_write(i2c_inst_t *inst, uint8_t command)
{
	int res = i2c_write_timeout_us(inst, address, &command, 1, false, 1000);
	if (res == PICO_ERROR_TIMEOUT)
	{
		// printf("I2C Write Timeout (0x%02X)\n", command);
		return -1;
	}
	else
	{
		// printf("I2C Write Success (0x%02X, %d)\n", command, res);
		return 0;
	}
}

int i2c_write(i2c_inst_t *inst, uint8_t command, uint16_t payload)
{
	uint8_t data[3] = {command, (uint8_t)(payload >> 8), (uint8_t)(payload & 0xFF)};

	int res = i2c_write_timeout_us(inst, address, data, 1, false, 1000);
	if (res == PICO_ERROR_TIMEOUT)
	{
		// printf("I2C Write Timeout (0x%02X)\n", command);
		return -1;
	}
	else
	{
		// printf("I2C Write Success (0x%02X, %d)\n", command, res);
		return 0;
	}
}

int i2c_read(i2c_inst_t *inst, uint8_t *status, uint16_t *data)
{
	uint8_t buffer[3];

	int res = i2c_read_timeout_us(inst, address, buffer, 3, false, 1000);
	if (res == PICO_ERROR_TIMEOUT)
	{
		// printf("I2C Read Timeout\n");
		return -1;
	}
	else if (res == PICO_ERROR_GENERIC)
	{
		// printf("I2C Read Error\n");
		return -1;
	}
	else
	{
		*status = buffer[0];
		*data = (buffer[1] << 8) | buffer[2];
		// printf("I2C Read Success (State: 0x%02X / Data: 0x%04X, %d)\n", *status, *data, res);
		return 0;
	}
}

int i2c_read(i2c_inst_t *inst, uint8_t *status, uint32_t *pressure, uint32_t *temperature)
{
	i2c_config(i2c1);

	uint8_t buffer[7];

	*pressure = 0;
	*temperature = 0;

	int res = i2c_read_timeout_us(inst, address, buffer, 7, false, 1000);
	if (res == PICO_ERROR_TIMEOUT)
	{
		printf("I2C Read Timeout\n");
		return -1;
	}
	else if (res == PICO_ERROR_GENERIC)
	{
		printf("I2C Read Error\n");
		return -1;
	}
	else
	{
		*status = buffer[0];
		*pressure = (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
		*temperature = (buffer[4] << 16) | (buffer[5] << 8) | buffer[6];
		printf("I2C Read Success (State: 0x%02X / Pressure: 0x%06X / Temperature: 0x%06X, %d)\n", *status, *pressure, *temperature, res);
		return 0;
	}
}

int main()
{
	stdio_init_all();

	if (cyw43_arch_init())
	{
		// printf("CYW43 Arch initialized\n");

		reset_usb_boot(0, 0);
		return 0;
	}

	sleep_ms(5000);
	for (int i = 0; i < 5; ++i)
	{
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
		sleep_ms(500);
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
		sleep_ms(500);
	}

	i2c_init(i2c1, 400 * 1000);

	gpio_set_function(I2C1_SDA_PIN, GPIO_FUNC_I2C);
	gpio_set_function(I2C1_SCL_PIN, GPIO_FUNC_I2C);

	gpio_pull_up(I2C1_SDA_PIN);
	gpio_pull_up(I2C1_SCL_PIN);

	// printf("I2C Hardware Initialized\n");

	while (true)
	{
		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

		uint8_t status = 0;
		uint32_t pressure = 0;
		uint32_t temperature = 0;

		i2c_read(i2c1, &status, &pressure, &temperature);

		cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
		sleep_ms(200);
	}
}